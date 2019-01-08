#ifndef __PACKET_H__
#define __PACKET_H__

typedef struct packet_queue_t
{
    AVPacketList *first_pkt;
    AVPacketList *last_pkt;
    int nb_packets;   // 队列中AVPacket的个数
    int size;         // 队列中AVPacket总的大小(字节数)
    SDL_mutex *mutex;
    SDL_cond *cond;
} packet_queue_t;

#if 0
typedef struct packet_queue_t {
    MyAVPacketList *first_pkt, *last_pkt;
    int nb_packets;                 // 队列中packet的数量
    int size;                       // 队列所占内存空间大小
    int64_t duration;               // 队列中所有packet总的播放时长
    int abort_request;
    int serial;                     // 播放序列，所谓播放序列就是一段连续的播放动作，一个seek操作会启动一段新的播放序列
    SDL_mutex *mutex;
    SDL_cond *cond;
} packet_queue_t;
#endif

void packet_queue_init(packet_queue_t *q);
int packet_queue_put(packet_queue_t *q, AVPacket *pkt);
int packet_queue_get(packet_queue_t *q, AVPacket *pkt, int block);
int packet_queue_put_nullpacket(packet_queue_t *q, int stream_index)

#define
