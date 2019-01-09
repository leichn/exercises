#include "video.h"
#include "packet.h"
#include "frame.h"

typedef struct {
    SDL_Window* screen; 
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect rect;
    SDL_Thread* thread;
    SDL_Event event;
}   sdl_video_t;

typedef struct {
    double frame_timer;             // 记录最后一帧播放的时刻
    sdl_video_t sdl;
}   player_video_t;

static player_video_t s_video_stat;

int video_init_decode()
{

}


// 从packet_queue中取一个packet，解码生成frame
static int decoder_decode_frame(AVCodecContext *p_codec_ctx, AVFrame *frame)
{
    int ret = AVERROR(EAGAIN);

    while (1)
    {
        AVPacket pkt;

        while (1)
        {
            //if (d->queue->abort_request)
            //    return -1;

            // 3. 从解码器接收frame
            // 3.1 一个视频packet含一个视频frame
            //     解码器缓存一定数量的packet后，才有解码后的frame输出
            //     frame输出顺序是按pts的顺序，如IBBPBBP
            //     frame->pkt_pos变量是此frame对应的packet在视频文件中的偏移地址，值同pkt.pos
            ret = avcodec_receive_frame(d->avctx, frame);
            if (ret < 0)
            {
                if (ret == AVERROR_EOF)
                {
                    printf("video avcodec_send_packet(): the decoder has been flushed\n");
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                else if (ret == AVERROR(EAGAIN))
                {
                    printf("video avcodec_send_packet(): input is not accepted in the current state\n");
                    break;
                }
                else
                {
                    printf("video avcodec_send_packet(): legitimate decoding errors\n");
                    return -1;
                }
            }
            else
            {
                frame->pts = frame->best_effort_timestamp;
                //frame->pts = frame->pkt_dts;

                return 1;   // 成功解码得到一个视频帧或一个音频帧，则返回
            }
        }

        // 1. 取出一个packet。使用pkt对应的serial赋值给d->pkt_serial
        if (packet_queue_get(d->queue, &pkt, true) < 0)
        {
            return -1;
        }

        // packet_queue中第一个总是flush_pkt。每次seek操作会插入flush_pkt，更新serial，开启新的播放序列
        if (pkt.data == flush_pkt.data)
        {
            // 复位解码器内部状态/刷新内部缓冲区。当seek操作或切换流时应调用此函数。
            avcodec_flush_buffers(d->avctx);
        }
        else
        {
            // 2. 将packet发送给解码器
            //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
            //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
            if (avcodec_send_packet(d->avctx, &pkt) == AVERROR(EAGAIN))
            {
                printf("Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
                d->packet_pending = 1;
                av_packet_move_ref(&d->pkt, &pkt);
            }

            av_packet_unref(&pkt);
        }
    }
}

// 将视频包解码得到视频帧，然后写入picture队列
int video_decode_thread(void *arg)
{
    AVCodecContext *p_codec_ctx = (AVCodecContext *)arg;

    AVFrame* p_frame = NULL;
    AVFrame* p_frm_yuv = NULL;
    AVPacket* p_packet = NULL;
    struct SwsContext*  sws_ctx = NULL;
    int buf_size;
    uint8_t* buffer = NULL;

    int ret = 0;
    int res = -1;
    
    p_packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    // 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
    p_frame = av_frame_alloc();
    if (p_frame == NULL)
    {
        printf("av_frame_alloc() for p_frame failed\n");
        res = -1;
        goto exit0;
    }

    while (1)
    {
        // 从队列中读出一包视频数据
        if (packet_queue_get(&s_video_pkt_queue, p_packet, 1) <= 0)
        {
            if (s_input_finished)
            {
                av_packet_unref(p_packet);
                p_packet = NULL;    // flush decoder
                printf("Flushing video decoder...\n");
            }
            else
            {
                av_packet_unref(p_packet);
                return;
            }
        }

        // 向解码器喂数据，一个packet可能是一个视频帧或多个音频帧，此处音频帧已被上一句滤掉
        ret = avcodec_send_packet(p_codec_ctx, p_packet);
        if (ret != 0)
        {
            if (ret == AVERROR_EOF)
            {
                printf("video avcodec_send_packet(): the decoder has been flushed\n");
            }
            else if (ret == AVERROR(EAGAIN))
            {
                printf("video avcodec_send_packet(): input is not accepted in the current state\n");
            }
            else if (ret == AVERROR(EINVAL))
            {
                printf("video avcodec_send_packet(): codec not opened, it is an encoder, or requires flush\n");
            }
            else if (ret == AVERROR(ENOMEM))
            {
                printf("video avcodec_send_packet(): failed to add packet to internal queue, or similar\n");
            }
            else
            {
                printf("video avcodec_send_packet(): legitimate decoding errors\n");
            }

			res = -1;
            goto exit5;
        }
        // 接收解码器输出的数据，此处只处理视频帧，每次接收一个packet，将之解码得到一个frame
        ret = avcodec_receive_frame(p_codec_ctx, p_frame);
        if (ret != 0)
        {
            if (ret == AVERROR_EOF)
            {
                printf("video avcodec_receive_frame(): the decoder has been fully flushed\n");
                s_vdecode_finished = true;
            }
            else if (ret == AVERROR(EAGAIN))
            {
                printf("video avcodec_receive_frame(): output is not available in this state - "
                        "user must try to send new input\n");
				continue;
            }
            else if (ret == AVERROR(EINVAL))
            {
                printf("video avcodec_receive_frame(): codec not opened, or it is an encoder\n");
            }
            else
            {
                printf("video avcodec_receive_frame(): legitimate decoding errors\n");
            }

            res = -1;
            goto exit6;
        }
    }

exit6:
    if (p_packet != NULL)
    {
        av_packet_unref(p_packet);
    }
exit5:
    sws_freeContext(sws_ctx); 
exit4:
    av_free(buffer);
exit3:
    av_frame_free(&p_frm_yuv);
exit2:
    av_frame_free(&p_frame);
exit1:
    avcodec_close(p_codec_ctx);
exit0:
    return res;
}

int video_init_playing()
{

}

static void video_image_display(VideoState *is)
{
    Frame *vp;
    Frame *sp = NULL;
    SDL_Rect rect;

    vp = frame_queue_peek_last(&is->pictq);

    calculate_display_rect(&rect, is->xleft, is->ytop, is->width, is->height, vp->width, vp->height, vp->sar);

    if (!vp->uploaded) {
        if (upload_texture(&is->vid_texture, vp->frame, &is->img_convert_ctx) < 0)
            return;
        vp->uploaded = 1;
        vp->flip_v = vp->frame->linesize[0] < 0;
    }

    set_sdl_yuv_conversion_mode(vp->frame);
    SDL_RenderCopyEx(renderer, is->vid_texture, NULL, &rect, 0, NULL, vp->flip_v ? SDL_FLIP_VERTICAL : 0);
    set_sdl_yuv_conversion_mode(NULL);
    if (sp) {
        int i;
        double xratio = (double)rect.w / (double)sp->width;
        double yratio = (double)rect.h / (double)sp->height;
        for (i = 0; i < sp->sub.num_rects; i++) {
            SDL_Rect *sub_rect = (SDL_Rect*)sp->sub.rects[i];
            SDL_Rect target = {.x = rect.x + sub_rect->x * xratio,
                               .y = rect.y + sub_rect->y * yratio,
                               .w = sub_rect->w * xratio,
                               .h = sub_rect->h * yratio};
            SDL_RenderCopy(renderer, is->sub_texture, sub_rect, &target);
        }
    }
}

/* display the current picture, if any */
static void video_display(VideoState *is)
{
    if (!is->width)
        video_open(is);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    video_image_display(is);
    SDL_RenderPresent(renderer);
}

/* called to display each frame */
static void video_refresh(void *opaque, double *remaining_time)
{
    VideoState *is = opaque;
    double time;

    Frame *sp, *sp2;

    // 视频播放
    if (is->video_st) {
retry:
        if (frame_queue_nb_remaining(&is->pictq) == 0) {    // 所有帧已显示
            // nothing to do, no picture to display in the queue
        } else {                                            // 有未显示帧
            double last_duration, duration, delay;
            Frame *vp, *lastvp;

            /* dequeue the picture */
            lastvp = frame_queue_peek_last(&is->pictq);     // 上一帧：上次已显示的帧
            vp = frame_queue_peek(&is->pictq);              // 当前帧：当前待显示的帧

            if (vp->serial != is->videoq.serial) {
                frame_queue_next(&is->pictq);
                goto retry;
            }

            // lastvp和vp不是同一播放序列(一个seek会开始一个新播放序列)，将frame_timer更新为当前时间
            if (lastvp->serial != vp->serial)
                is->frame_timer = av_gettime_relative() / 1000000.0;

            // 暂停处理：不停播放上一帧图像
            if (is->paused)
                goto display;

            /* compute nominal last_duration */
            last_duration = vp_duration(is, lastvp, vp);        // 上一帧播放时长：vp->pts - lastvp->pts
            delay = compute_target_delay(last_duration, is);    // 根据视频时钟和同步时钟的差值，计算delay值

            time= av_gettime_relative()/1000000.0;
            // 当前帧播放时刻(is->frame_timer+delay)大于当前时刻(time)，表示播放时刻未到
            if (time < is->frame_timer + delay) {
                // 播放时刻未到，则更新刷新时间remaining_time为当前时刻到下一播放时刻的时间差
                *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
                // 播放时刻未到，则不更新rindex，把上一帧lastvp再播放一遍
                goto display;
            }

            // 更新frame_timer值
            is->frame_timer += delay;
            // 校正frame_timer值：若frame_timer落后于当前系统时间太久(超过最大同步域值)，则更新为当前系统时间
            if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
                is->frame_timer = time;

            SDL_LockMutex(is->pictq.mutex);
            if (!isnan(vp->pts))
                update_video_pts(is, vp->pts, vp->pos, vp->serial); // 更新视频时钟：时间戳、时钟时间
            SDL_UnlockMutex(is->pictq.mutex);

            // 是否要丢弃未能及时播放的视频帧
            if (frame_queue_nb_remaining(&is->pictq) > 1) {         // 队列中未显示帧数>1(只有一帧则不考虑丢帧)
                Frame *nextvp = frame_queue_peek_next(&is->pictq);  // 下一帧：下一待显示的帧
                duration = vp_duration(is, vp, nextvp);             // 当前帧vp播放时长 = nextvp->pts - vp->pts
                // 当前帧vp未能及时播放，即下一帧播放时刻(is->frame_timer+duration)小于当前系统时刻(time)
                if (time > is->frame_timer + duration)
                {
                    is->frame_drops_late++;         // framedrop丢帧处理有两处：1) packet入队列前，2) frame未及时显示(此处)
                    frame_queue_next(&is->pictq);   // 删除上一帧已显示帧，即删除lastvp，读指针加1(从lastvp更新到vp)
                    goto retry;
                }
            }

            // 删除当前读指针元素，读指针+1。若未丢帧，读指针从lastvp更新到vp；若有丢帧，读指针从vp更新到nextvp
            frame_queue_next(&is->pictq);
        }
display:
        /* display picture */
        //-if (is->force_refresh && is->pictq.rindex_shown)
            video_display(is);                      // 取出当前帧vp(若有丢帧是nextvp)进行播放
    }
}

static void video_playing_thread(void *arg)
{
    double remaining_time = 0.0;
    sdl_video_t sdl;

    // 1. 创建SDL窗口，SDL 2.0支持多窗口
    //    SDL_Window即运行程序后弹出的视频窗口，同SDL 1.x中的SDL_Surface
    sdl.screen = SDL_CreateWindow("simple ffplayer", 
                              SDL_WINDOWPOS_UNDEFINED,// 不关心窗口X坐标
                              SDL_WINDOWPOS_UNDEFINED,// 不关心窗口Y坐标
                              p_codec_ctx->width, 
                              p_codec_ctx->height,
                              SDL_WINDOW_OPENGL
                              );
    if (sdl.screen == NULL)
    {  
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit5;
    }

    // 2. 创建SDL_Renderer
    //    SDL_Renderer：渲染器
    sdl.renderer = SDL_CreateRenderer(sdl.screen, -1, 0);
    if (sdl.renderer == NULL)
    {  
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit5;
    }

    // B3. 创建SDL_Texture
    //     一个SDL_Texture对应一帧YUV数据，同SDL 1.x中的SDL_Overlay
    sdl.texture = SDL_CreateTexture(sdl.renderer, 
                                    SDL_PIXELFORMAT_IYUV, 
                                    SDL_TEXTUREACCESS_STREAMING,
                                    p_codec_ctx->width,
                                    p_codec_ctx->height
                                    );
    if (sdl.texture == NULL)
    {  
        printf("SDL_CreateTexture() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit5;
    }

    // B4. SDL_Rect赋值
    sdl.rect.x = 0;
    sdl.rect.y = 0;
    sdl.rect.w = p_codec_ctx->width;
    sdl.rect.h = p_codec_ctx->height;

    while (1)
    {
        if (remaining_time > 0.0)
        {
            av_usleep((int64_t)(remaining_time * 1000000.0));
        }
        remaining_time = REFRESH_RATE;
        // 立即显示当前帧，或延时remaining_time后再显示
        video_refresh(is, &remaining_time);
    }
}

int open_video_stream(const AVStream* p_stream)
{
    AVCodecParameters* p_codec_par = NULL;
    AVCodec* p_codec = NULL;
    int ret;

    // 1. 为视频流构建解码器AVCodecContext
    // 1.1 获取解码器参数AVCodecParameters
    p_codec_par = p_stream->codecpar;

    // 1.2 获取解码器
    p_codec = avcodec_find_decoder(p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        printf("Cann't find codec!\n");
        return -1;
    }

    // 1.3 构建解码器AVCodecContext
    // 1.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        printf("avcodec_alloc_context3() failed %d\n", ret);
        return -1;
    }
    // 1.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0)
    {
        printf("avcodec_parameters_to_context() failed %d\n", ret);
        return -1;
    }
    // 1.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2() failed %d\n", ret);
        return -1;
    }
    
    #if 0
    int temp_num = p_fmt_ctx->streams[stream_idx]->avg_frame_rate.num;
    int temp_den = p_fmt_ctx->streams[stream_idx]->avg_frame_rate.den;
    int frame_rate = (temp_den > 0) ? temp_num/temp_den : 25;
    int interval = (temp_num > 0) ? (temp_den*1000)/temp_num : 40;

    printf("frame rate %d FPS, refresh interval %d ms\n", frame_rate, interval);
    #endif

    // 2. 创建视频解码线程
    SDL_CreateThread(video_decode_thread, "video decode thread", p_codec_ctx);

    return 0;
}

int open_video(const AVStream* p_stream)
{
    open_video_stream(p_stream);
    open_video_playing();

    return 0;
}

