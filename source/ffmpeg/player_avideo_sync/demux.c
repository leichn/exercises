#include "demux.h"
#include "packet.h"

static int decode_interrupt_cb(void *ctx)
{
    player_stat_t *is = ctx;
    return is->abort_request;
}

int demux_init(const char *p_input_file, player_demux_t *demux)
{
    player_stat_t *is = arg;
    AVFormatContext *ic = NULL;
    int err, i, ret;
    int a_idx;
    int v_idx;

    ic = avformat_alloc_context();
    if (!ic)
    {
        printf("Could not allocate context.\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    // 中断回调机制。为底层I/O层提供一个处理接口，比如中止IO操作。
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;

    // 1. 构建AVFormatContext
    // 1.1 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    err = avformat_open_input(&ic, is->filename, NULL, NULL);
    if (err < 0)
    {
        printf("avformat_open_input() failed %d\n", err);
        ret = -1;
        goto fail;
    }
    demux->ic = ic;

    // 1.2 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入p_fmt_ctx->streams
    //     ic->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    ret = avformat_find_stream_info(ic, NULL);
    if (ret < 0)
    {
        printf("avformat_find_stream_info() failed %d\n", err);
        ret = -1;
        goto fail;
    }

    // 2. 查找第一个音频流/视频流
    a_idx = -1;
    v_idx = -1;
    for (i=0; i<ic->nb_streams; i++)
    {
        if ((ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) &&
            (a_idx == -1))
        {
            a_idx = i;
            printf("Find a audio stream, index %d\n", a_idx);
            // 3. 打开音频流
            open_audio_stream(ic, a_idx);
        }
        if ((ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) &&
            (v_idx == -1))
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
            // 3. 打开视频流
            open_video_stream(ic, v_idx);
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
        if (ic && !demux->ic)
        {
            avformat_close_input(&ic);
        }

        return ret;
    }

    demux->aud_idx = a_idx;
    demux->vid_idx = v_idx;

    return 0;
}

int demux_deinit()
{

}


/* this thread gets the stream from the disk or the network */
int demux_thread(void *arg)
{
    player_demux_t *is = (player_demux_t *)arg;
    AVFormatContext *ic = NULL;
    int err, i, ret;
    AVPacket pkt1, *pkt = &pkt1;

    SDL_mutex *wait_mutex = SDL_CreateMutex();
    int64_t pkt_ts;

    // 4. 解复用处理
    for (;;) {

#if 0
        /* if the queue are full, no need to read more */
        if (infinite_buffer<1 &&
              (is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE
            || (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
                stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
                stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq)))) {
            /* wait 10 ms */
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
#endif

        // 4.1 从输入文件中读取一个packet
        ret = av_read_frame(is->p_ic, pkt);
        if (ret < 0)
        {
            if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof)
            {
                // 输入文件已读完，则往packet队列中发送NULL packet，以冲洗(flush)解码器，否则解码器中缓存的帧取不出来
                if (is->video_stream >= 0)
                {
                    packet_queue_put_nullpacket(is->p_vid_pkt_q, is->vid_idx);
                }
                if (is->audio_stream >= 0)
                {
                    packet_queue_put_nullpacket(is->p_aud_pkt_q, is->aud_idx);
                }
                is->eof = 1;
            }

            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        else
        {
            is->eof = 0;
        }
        
        // 4.3 根据当前packet类型(音频、视频、字幕)，将其存入对应的packet队列
        if (pkt->stream_index == is->aud_idx)
        {
            packet_queue_put(&is->p_aud_pkt_q, pkt);
        }
        else if (pkt->stream_index == is->vid_idx)
        {
            packet_queue_put(&is->p_vid_pkt_q, pkt);
        }
        else
        {
            av_packet_unref(pkt);
        }
    }

    ret = 0;

    if (ret != 0) {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    
    SDL_DestroyMutex(wait_mutex);
    return 0;
}


int open_demux(const char *p_input_file, player_demux_t *demux)
{
    if (demux_init(p_input_file, demux) != 0)
    {
        printf("demux_init() failed\n");
        return -1;
    }

    SDL_Thread *tid = SDL_CreateThread(demux_thread, "demux_thread", demux);
    if (tid == NULL)
    {
        printf("SDL_CreateThread() failed: %s\n", SDL_GetError());
        return -1;
    }
}