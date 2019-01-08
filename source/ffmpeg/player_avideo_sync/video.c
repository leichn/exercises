

// 将视频包解码得到视频帧，然后写入picture队列
int video_thread(void *arg)
{
    AVCodecContext *p_codec_ctx = (AVCodecContext *)arg;

    AVFrame* p_frm_raw = NULL;
    AVFrame* p_frm_yuv = NULL;
    AVPacket* p_packet = NULL;
    struct SwsContext*  sws_ctx = NULL;
    int buf_size;
    uint8_t* buffer = NULL;
    SDL_Window* screen; 
    SDL_Renderer* sdl_renderer;
    SDL_Texture* sdl_texture;
    SDL_Rect sdl_rect;
    SDL_Thread* sdl_thread;
    SDL_Event sdl_event;

    int ret = 0;
    int res = -1;
    
    p_packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    // A1. 分配AVFrame
    // A1.1 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
    p_frm_raw = av_frame_alloc();
    if (p_frm_raw == NULL)
    {
        printf("av_frame_alloc() for p_frm_raw failed\n");
        res = -1;
        goto exit0;
    }
    p_frm_yuv = av_frame_alloc();
    if (p_frm_yuv == NULL)
    {
        printf("av_frame_alloc() for p_frm_raw failed\n");
        res = -1;
        goto exit1;
    }

    // A1.2 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
    //     p_frm_raw的data_buffer由av_read_frame()分配，因此不需手工分配
    //     p_frm_yuv的data_buffer无处分配，因此在此处手工分配
    buf_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
                                        p_codec_ctx->width, 
                                        p_codec_ctx->height, 
                                        1
                                        );
    // buffer将作为p_frm_yuv的视频数据缓冲区
    buffer = (uint8_t *)av_malloc(buf_size);
    if (buffer == NULL)
    {
        printf("av_malloc() for buffer failed\n");
        res = -1;
        goto exit2;
    }
    // 使用给定参数设定p_frm_yuv->data和p_frm_yuv->linesize
    ret = av_image_fill_arrays(p_frm_yuv->data,     // dst data[]
                               p_frm_yuv->linesize, // dst linesize[]
                               buffer,              // src buffer
                               AV_PIX_FMT_YUV420P,  // pixel format
                               p_codec_ctx->width,  // width
                               p_codec_ctx->height, // height
                               1                    // align
                               );
    if (ret < 0)
    {
        printf("av_image_fill_arrays() failed %d\n", ret);
        res = -1;
        goto exit3;
    }

    // A2. 初始化SWS context，用于后续图像转换
    sws_ctx = sws_getContext(p_codec_ctx->width,    // src width
                             p_codec_ctx->height,   // src height
                             p_codec_ctx->pix_fmt,  // src format
                             p_codec_ctx->width,    // dst width
                             p_codec_ctx->height,   // dst height
                             AV_PIX_FMT_YUV420P,    // dst format
                             SWS_BICUBIC,           // flags
                             NULL,                  // src filter
                             NULL,                  // dst filter
                             NULL                   // param
                             );
    if (sws_ctx == NULL)
    {
        printf("sws_getContext() failed\n");
        res = -1;
        goto exit4;
    }

    // B1. 创建SDL窗口，SDL 2.0支持多窗口
    //     SDL_Window即运行程序后弹出的视频窗口，同SDL 1.x中的SDL_Surface
    screen = SDL_CreateWindow("simple ffplayer", 
                              SDL_WINDOWPOS_UNDEFINED,// 不关心窗口X坐标
                              SDL_WINDOWPOS_UNDEFINED,// 不关心窗口Y坐标
                              p_codec_ctx->width, 
                              p_codec_ctx->height,
                              SDL_WINDOW_OPENGL
                              );
    if (screen == NULL)
    {  
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit5;
    }

    // B2. 创建SDL_Renderer
    //     SDL_Renderer：渲染器
    sdl_renderer = SDL_CreateRenderer(screen, -1, 0);
    if (sdl_renderer == NULL)
    {  
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit5;
    }

    // B3. 创建SDL_Texture
    //     一个SDL_Texture对应一帧YUV数据，同SDL 1.x中的SDL_Overlay
    sdl_texture = SDL_CreateTexture(sdl_renderer, 
                                    SDL_PIXELFORMAT_IYUV, 
                                    SDL_TEXTUREACCESS_STREAMING,
                                    p_codec_ctx->width,
                                    p_codec_ctx->height
                                    );
    if (sdl_texture == NULL)
    {  
        printf("SDL_CreateTexture() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit5;
    }

    // B4. SDL_Rect赋值
    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = p_codec_ctx->width;
    sdl_rect.h = p_codec_ctx->height;

    while (1)
    {
        if (s_vdecode_finished)
        {
            break;
        }
        
        // A3. 从队列中读出一包视频数据
        if (packet_queue_pop(&s_video_pkt_queue, p_packet, 1) <= 0)
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

        // A4. 视频解码：packet ==> frame
        // A4.1 向解码器喂数据，一个packet可能是一个视频帧或多个音频帧，此处音频帧已被上一句滤掉
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
        // A4.2 接收解码器输出的数据，此处只处理视频帧，每次接收一个packet，将之解码得到一个frame
        ret = avcodec_receive_frame(p_codec_ctx, p_frm_raw);
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
        
        // A5. 图像转换：p_frm_raw->data ==> p_frm_yuv->data
        // 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
        // plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
        // slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
        // stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
        // AVFrame.*data[]: 每个数组元素指向对应plane
        // AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数
        sws_scale(sws_ctx,                                  // sws context
                  (const uint8_t *const *)p_frm_raw->data,  // src slice
                  p_frm_raw->linesize,                      // src stride
                  0,                                        // src slice y
                  p_codec_ctx->height,                      // src slice height
                  p_frm_yuv->data,                          // dst planes
                  p_frm_yuv->linesize                       // dst strides
                  );
        
        // B5. 使用新的YUV像素数据更新SDL_Rect
        SDL_UpdateYUVTexture(sdl_texture,                   // sdl texture
                             &sdl_rect,                     // sdl rect
                             p_frm_yuv->data[0],            // y plane
                             p_frm_yuv->linesize[0],        // y pitch
                             p_frm_yuv->data[1],            // u plane
                             p_frm_yuv->linesize[1],        // u pitch
                             p_frm_yuv->data[2],            // v plane
                             p_frm_yuv->linesize[2]         // v pitch
                             );
        
        // B6. 使用特定颜色清空当前渲染目标
        SDL_RenderClear(sdl_renderer);
        // B9. 使用部分图像数据(texture)更新当前渲染目标
        SDL_RenderCopy(sdl_renderer,                        // sdl renderer
                       sdl_texture,                         // sdl texture
                       NULL,                                // src rect, if NULL copy texture
                       &sdl_rect                            // dst rect
                       );
        
        // B7. 执行渲染，更新屏幕显示
        SDL_RenderPresent(sdl_renderer);
        if (p_packet != NULL)
        {
            av_packet_unref(p_packet);
        }

        SDL_WaitEvent(&sdl_event);
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
    av_frame_free(&p_frm_raw);
exit1:
    avcodec_close(p_codec_ctx);
exit0:
    return res;
}

int open_video_stream(AVFormatContext* p_fmt_ctx, AVCodecContext* p_codec_ctx, int steam_idx)
{
    AVCodecParameters* p_codec_par = NULL;
    AVCodec* p_codec = NULL;
    SDL_Window* screen; 
    SDL_Renderer* sdl_renderer;
    SDL_Texture* sdl_texture;
    SDL_Rect sdl_rect;
    SDL_Thread* sdl_thread;
    SDL_Event sdl_event;

    int ret;

    packet_queue_init(&s_video_pkt_queue);

    // 1. 为视频流构建解码器AVCodecContext
    // 1.1 获取解码器参数AVCodecParameters
    p_codec_par = p_fmt_ctx->streams[steam_idx]->codecpar;

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
    
    int temp_num = p_fmt_ctx->streams[steam_idx]->avg_frame_rate.num;
    int temp_den = p_fmt_ctx->streams[steam_idx]->avg_frame_rate.den;
    int frame_rate = (temp_den > 0) ? temp_num/temp_den : 25;
    int interval = (temp_num > 0) ? (temp_den*1000)/temp_num : 40;

    printf("frame rate %d FPS, refresh interval %d ms\n", frame_rate, interval);

    // 2. 创建视频解码定时刷新线程，此线程为SDL内部线程，调用指定的回调函数
    SDL_AddTimer(interval, sdl_time_cb_refresh, NULL);

    // 3. 创建视频解码线程
    SDL_CreateThread(video_thread, "video thread", p_codec_ctx);

    return 0;
}

