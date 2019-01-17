#include "packet.h"

int packet_queue_init(packet_queue_t *q)
{
    memset(q, 0, sizeof(packet_queue_t));
    q->mutex = SDL_CreateMutex();
    if (!q->mutex)
    {
        printf("SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->cond = SDL_CreateCond();
    if (!q->cond)
    {
        printf("SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->abort_request = 0;
    return 0;
}


// 写队列尾部。pkt是一包还未解码的音频数据
int packet_queue_put(packet_queue_t *q, AVPacket *pkt)
{
    AVPacketList *pkt_list;
    
    if (av_packet_make_refcounted(pkt) < 0)
    {
        printf("[pkt] is not refrence counted\n");
        return -1;
    }
    pkt_list = av_malloc(sizeof(AVPacketList));
    if (!pkt_list)
    {
        return -1;
    }
    
    pkt_list->pkt = *pkt;
    pkt_list->next = NULL;

    SDL_LockMutex(q->mutex);

    if (!q->last_pkt)   // 队列为空
    {
        q->first_pkt = pkt_list;
    }
    else
    {
        q->last_pkt->next = pkt_list;
    }
    q->last_pkt = pkt_list;
    q->nb_packets++;
    q->size += pkt_list->pkt.size;
    // 发个条件变量的信号：重启等待q->cond条件变量的一个线程
    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
    return 0;
}

// 读队列头部。
int packet_queue_get(packet_queue_t *q, AVPacket *pkt, int block)
{
    AVPacketList *p_pkt_node;
    int ret;

    SDL_LockMutex(q->mutex);

    while (1)
    {
        p_pkt_node = q->first_pkt;
        if (p_pkt_node)             // 队列非空，取一个出来
        {
            q->first_pkt = p_pkt_node->next;
            if (!q->first_pkt)
            {
                q->last_pkt = NULL;
            }
            q->nb_packets--;
            q->size -= p_pkt_node->pkt.size;
            *pkt = p_pkt_node->pkt;
            av_free(p_pkt_node);
            ret = 1;
            break;
        }
        else if (!block)            // 队列空且阻塞标志无效，则立即退出
        {
            ret = 0;
            break;
        }
        else                        // 队列空且阻塞标志有效，则等待
        {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

int packet_queue_put_nullpacket(packet_queue_t *q, int stream_index)
{
    AVPacket pkt1, *pkt = &pkt1;
    av_init_packet(pkt);
    pkt->data = NULL;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

void packet_queue_flush(packet_queue_t *q)
{
    AVPacketList *pkt, *pkt1;

    SDL_LockMutex(q->mutex);
    for (pkt = q->first_pkt; pkt; pkt = pkt1) {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    q->duration = 0;
    SDL_UnlockMutex(q->mutex);
}

void packet_queue_destroy(packet_queue_t *q)
{
    packet_queue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

void packet_queue_abort(packet_queue_t *q)
{
    SDL_LockMutex(q->mutex);

    q->abort_request = 1;

    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
}

