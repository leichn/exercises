#include "demux.h"
#include "packet.h"

static int decode_interrupt_cb(void *ctx)
{
    player_stat_t *is = ctx;
    return is->abort_request;
}

static int demux_init(player_stat_t *is)
{
    AVFormatContext *p_fmt_ctx = NULL;
    int err, i, ret;
    int a_idx;
    int v_idx;

    p_fmt_ctx = avformat_alloc_context();
    if (!p_fmt_ctx)
    {
        printf("Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    // 中断回调机制。为底层I/O层提供一个处理接口，比如中止IO操作。
    p_fmt_ctx->interrupt_callback.callback = decode_interrupt_cb;
    p_fmt_ctx->interrupt_callback.opaque = is;

    // 1. 构建AVFormatContext
    // 1.1 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    err = avformat_open_input(&p_fmt_ctx, is->filename, NULL, NULL);
    if (err < 0)
    {
        printf("avformat_open_input() failed %d\n", err);
        ret = -1;
        goto fail;
    }
    is->p_fmt_ctx = p_fmt_ctx;

    // 1.2 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入p_fmt_ctx->streams
    //     ic->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    err = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (err < 0)
    {
        printf("avformat_find_stream_info() failed %d\n", err);
        ret = -1;
        goto fail;
    }

    // 2. 查找第一个音频流/视频流
    a_idx = -1;
    v_idx = -1;
    for (i=0; i<(int)p_fmt_ctx->nb_streams; i++)
    {
        if ((p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) &&
            (a_idx == -1))
        {
            a_idx = i;
            printf("Find a audio stream, index %d\n", a_idx);
        }
        if ((p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) &&
            (v_idx == -1))
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
        }
        if (a_idx != -1 && v_idx != -1)
        {
            break;
        }
    }
    if (a_idx == -1 && v_idx == -1)
    {
        printf("Cann't find any audio/video stream\n");
        ret = -1;
 fail:
        if (p_fmt_ctx != NULL)
        {
            avformat_close_input(&p_fmt_ctx);
        }
        return ret;
    }

    is->audio_idx = a_idx;
    is->video_idx = v_idx;
    is->p_audio_stream = p_fmt_ctx->streams[a_idx];
    is->p_video_stream = p_fmt_ctx->streams[v_idx];

    return 0;
}

int demux_deinit()
{
    return 0;
}

static int stream_has_enough_packets(AVStream *st, int stream_id, packet_queue_t *queue)
{
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

/* this thread gets the stream from the disk or the network */
static int demux_thread(void *arg)
{
    player_stat_t *is = (player_stat_t *)arg;
    AVFormatContext *p_fmt_ctx = is->p_fmt_ctx;
    int ret;
    AVPacket pkt1, *pkt = &pkt1;

    SDL_mutex *wait_mutex = SDL_CreateMutex();

    printf("demux_thread running...\n");

    // 4. 解复用处理
    while (1)
    {
        if (is->abort_request)
        {
            break;
        }
        
        /* if the queue are full, no need to read more */
        if (is->audio_pkt_queue.size + is->video_pkt_queue.size > MAX_QUEUE_SIZE ||
            (stream_has_enough_packets(is->p_audio_stream, is->audio_idx, &is->audio_pkt_queue) &&
             stream_has_enough_packets(is->p_video_stream, is->video_idx, &is->video_pkt_queue)))
        {
            /* wait 10 ms */
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }

        // 4.1 从输入文件中读取一个packet
        ret = av_read_frame(is->p_fmt_ctx, pkt);
        if (ret < 0)
        {
            if ((ret == AVERROR_EOF))// || avio_feof(ic->pb)) && !is->eof)
            {
                // 输入文件已读完，则往packet队列中发送NULL packet，以冲洗(flush)解码器，否则解码器中缓存的帧取不出来
                if (is->video_idx >= 0)
                {
                    packet_queue_put_nullpacket(&is->video_pkt_queue, is->video_idx);
                }
                if (is->audio_idx >= 0)
                {
                    packet_queue_put_nullpacket(&is->audio_pkt_queue, is->audio_idx);
                }
            }

            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        
        // 4.3 根据当前packet类型(音频、视频、字幕)，将其存入对应的packet队列
        if (pkt->stream_index == is->audio_idx)
        {
            packet_queue_put(&is->audio_pkt_queue, pkt);
        }
        else if (pkt->stream_index == is->video_idx)
        {
            packet_queue_put(&is->video_pkt_queue, pkt);
        }
        else
        {
            av_packet_unref(pkt);
        }
    }

    ret = 0;

    if (ret != 0)
    {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    
    SDL_DestroyMutex(wait_mutex);
    return 0;
}

int open_demux(player_stat_t *is)
{
    if (demux_init(is) != 0)
    {
        printf("demux_init() failed\n");
        return -1;
    }

    is->read_tid = SDL_CreateThread(demux_thread, "demux_thread", is);
    if (is->read_tid == NULL)
    {
        printf("SDL_CreateThread() failed: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}