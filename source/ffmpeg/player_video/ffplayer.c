/*******************************************************************************
 * ffplayer.c
 *
 * history:
 *   2018-11-27 - [lei]     Create file: a simplest ffmpeg player
 *   2018-11-29 - [lei]     Refresh decoding thread with SDL event 
 *
 * details:
 *   A simple ffmpeg player.
 *
 * refrence:
 *   1. https://blog.csdn.net/leixiaohua1020/article/details/38868499
 *   2. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial01.html
 *   3. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial02.html
 *******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

static bool s_playing_exit = false;
static bool s_playing_pause = false;

// 按照opaque传入的播放帧率参数，按固定间隔时间发送刷新事件
int sdl_thread_handle_refreshing(void *opaque)
{
    SDL_Event sdl_event;

    int frame_rate = *((int *)opaque);
    int interval = (frame_rate > 0) ? 1000/frame_rate : 40;

    printf("frame rate %d FPS, refresh interval %d ms\n", frame_rate, interval);

    while (!s_playing_exit)
    {
        if (!s_playing_pause)
        {
            sdl_event.type = SDL_USEREVENT_REFRESH;
            SDL_PushEvent(&sdl_event);
        }
        SDL_Delay(interval);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    // Initalizing these to NULL prevents segfaults!
    AVFormatContext*    p_fmt_ctx = NULL;
    AVCodecContext*     p_codec_ctx = NULL;
    AVCodecParameters*  p_codec_par = NULL;
    AVCodec*            p_codec = NULL;
    AVFrame*            p_frm_raw = NULL;        // 帧，由包解码得到原始帧
    AVFrame*            p_frm_yuv = NULL;        // 帧，由原始帧色彩转换得到
    AVPacket*           p_packet = NULL;         // 包，从流中读出的一段数据
    struct SwsContext*  sws_ctx = NULL;
    int                 buf_size;
    uint8_t*            buffer = NULL;
    int                 i;
    int                 v_idx;
    int                 ret;
    int                 res;
    int                 frame_rate;
    SDL_Window*         screen; 
    SDL_Renderer*       sdl_renderer;
    SDL_Texture*        sdl_texture;
    SDL_Rect            sdl_rect;
    SDL_Thread*         sdl_thread;
    SDL_Event           sdl_event;

    res = 0;

    if (argc < 2)
    {
        printf("Please provide a movie file\n");
        return -1;
    }

    // 初始化libavformat(所有格式)，注册所有复用器/解复用器
    // av_register_all();   // 已被申明为过时的，直接不再使用即可

    // A1. 打开视频文件：读取文件头，将文件格式信息存储在"fmt context"中
    ret = avformat_open_input(&p_fmt_ctx, argv[1], NULL, NULL);
    if (ret != 0)
    {
        printf("avformat_open_input() failed %d\n", ret);
        res = -1;
        goto exit0;
    }

    // A2. 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入pFormatCtx->streams
    //     p_fmt_ctx->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    ret = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (ret < 0)
    {
        printf("avformat_find_stream_info() failed %d\n", ret);
        res = -1;
        goto exit1;
    }

    // 将文件相关信息打印在标准错误设备上
    av_dump_format(p_fmt_ctx, 0, argv[1], 0);

    // A3. 查找第一个视频流
    v_idx = -1;
    for (i=0; i<p_fmt_ctx->nb_streams; i++)
    {
        if (p_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            v_idx = i;
            printf("Find a video stream, index %d\n", v_idx);
            frame_rate = p_fmt_ctx->streams[i]->avg_frame_rate.num /
                         p_fmt_ctx->streams[i]->avg_frame_rate.den;
            break;
        }
    }
    if (v_idx == -1)
    {
        printf("Cann't find a video stream\n");
        res = -1;
        goto exit1;
    }

    // A5. 为视频流构建解码器AVCodecContext

    // A5.1 获取解码器参数AVCodecParameters
    p_codec_par = p_fmt_ctx->streams[v_idx]->codecpar;

    // A5.2 获取解码器
    p_codec = avcodec_find_decoder(p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        printf("Cann't find codec!\n");
        res = -1;
        goto exit1;
    }

    // A5.3 构建解码器AVCodecContext
    // A5.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);
    if (p_codec_ctx == NULL)
    {
        printf("avcodec_alloc_context3() failed %d\n", ret);
        res = -1;
        goto exit1;
    }
    // A5.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0)
    {
        printf("avcodec_parameters_to_context() failed %d\n", ret);
        res = -1;
        goto exit2;
    }
    // A5.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2() failed %d\n", ret);
        res = -1;
        goto exit2;
    }

    // A6. 分配AVFrame
    // A6.1 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
    p_frm_raw = av_frame_alloc();
    if (p_frm_raw == NULL)
    {
        printf("av_frame_alloc() for p_frm_raw failed\n");
        res = -1;
        goto exit2;
    }
    p_frm_yuv = av_frame_alloc();
    if (p_frm_yuv == NULL)
    {
        printf("av_frame_alloc() for p_frm_raw failed\n");
        res = -1;
        goto exit3;
    }

    // A6.2 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
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
        goto exit4;
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
        goto exit5;
    }

    // A7. 初始化SWS context，用于后续图像转换
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
        goto exit6;
    }

    // B1. 初始化SDL子系统：缺省(事件处理、文件IO、线程)、视频、音频、定时器
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    {  
        printf("SDL_Init() failed: %s\n", SDL_GetError()); 
        res = -1;
        goto exit6;
    }

    // B2. 创建SDL窗口，SDL 2.0支持多窗口
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
        goto exit7;
    }

    // B3. 创建SDL_Renderer
    //     SDL_Renderer：渲染器
    sdl_renderer = SDL_CreateRenderer(screen, -1, 0);
    if (sdl_renderer == NULL)
    {  
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit7;
    }

    // B4. 创建SDL_Texture
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
        goto exit7;
    }

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = p_codec_ctx->width;
    sdl_rect.h = p_codec_ctx->height;

    p_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (p_packet == NULL)
    {  
        printf("SDL_CreateThread() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit7;
    }

    // B5. 创建定时刷新事件线程，按照预设帧率产生刷新事件
    sdl_thread = SDL_CreateThread(sdl_thread_handle_refreshing, NULL, (void *)&frame_rate);
    if (sdl_thread == NULL)
    {  
        printf("SDL_CreateThread() failed: %s\n", SDL_GetError());  
        res = -1;
        goto exit8;
    }

    while (1)
    {
        // B6. 等待刷新事件
        SDL_WaitEvent(&sdl_event);

        if (sdl_event.type == SDL_USEREVENT_REFRESH)
        {
            // A8. 从视频文件中读取一个packet
            //     packet可能是视频帧、音频帧或其他数据，解码器只会解码视频帧或音频帧，非音视频数据并不会被
            //     扔掉、从而能向解码器提供尽可能多的信息
            //     对于视频来说，一个packet只包含一个frame
            //     对于音频来说，若是帧长固定的格式则一个packet可包含整数个frame，
            //                   若是帧长可变的格式则一个packet只包含一个frame
            while (av_read_frame(p_fmt_ctx, p_packet) == 0)
            {
                if (p_packet->stream_index == v_idx)  // 取到一帧视频帧，则退出
                {
                    break;
                }
            }

            // A9. 视频解码：packet ==> frame
            // A9.1 向解码器喂数据，一个packet可能是一个视频帧或多个音频帧，此处音频帧已被上一句滤掉
            ret = avcodec_send_packet(p_codec_ctx, p_packet);
            if (ret != 0)
            {
                printf("avcodec_send_packet() failed %d\n", ret);
                res = -1;
                goto exit8;
            }
            // A9.2 接收解码器输出的数据，此处只处理视频帧，每次接收一个packet，将之解码得到一个frame
            ret = avcodec_receive_frame(p_codec_ctx, p_frm_raw);
            if (ret != 0)
            {
                if (ret == AVERROR_EOF)
                {
                    printf("avcodec_receive_frame(): the decoder has been fully flushed\n");
                }
                else if (ret == AVERROR(EAGAIN))
                {
                    printf("avcodec_receive_frame(): output is not available in this state - "
                            "user must try to send new input\n");
                }
                else if (ret == AVERROR(EINVAL))
                {
                    printf("avcodec_receive_frame(): codec not opened, or it is an encoder\n");
                }
                else
                {
                    printf("avcodec_receive_frame(): legitimate decoding errors\n");
                }
                res = -1;
                goto exit8;
            }

            // A10. 图像转换：p_frm_raw->data ==> p_frm_yuv->data
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

            // B7. 使用新的YUV像素数据更新SDL_Rect
            SDL_UpdateYUVTexture(sdl_texture,                   // sdl texture
                                 &sdl_rect,                     // sdl rect
                                 p_frm_yuv->data[0],            // y plane
                                 p_frm_yuv->linesize[0],        // y pitch
                                 p_frm_yuv->data[1],            // u plane
                                 p_frm_yuv->linesize[1],        // u pitch
                                 p_frm_yuv->data[2],            // v plane
                                 p_frm_yuv->linesize[2]         // v pitch
                                 );

            // B8. 使用特定颜色清空当前渲染目标
            SDL_RenderClear(sdl_renderer);
            // B9. 使用部分图像数据(texture)更新当前渲染目标
            SDL_RenderCopy(sdl_renderer,                        // sdl renderer
                           sdl_texture,                         // sdl texture
                           NULL,                                // src rect, if NULL copy texture
                           &sdl_rect                            // dst rect
                           );

            // B10. 执行渲染，更新屏幕显示
            SDL_RenderPresent(sdl_renderer);

            av_packet_unref(p_packet);
        }
        else if (sdl_event.type == SDL_KEYDOWN)
        {
            if (sdl_event.key.keysym.sym == SDLK_SPACE)
            {
                // 用户按空格键，暂停/继续状态切换
                s_playing_pause = !s_playing_pause;
                printf("player %s\n", s_playing_pause ? "pause" : "continue");
            }
        }
        else if (sdl_event.type == SDL_QUIT)
        {
            // 用户按下关闭窗口按钮
            printf("SDL event QUIT\n");
            s_playing_exit = true;
            break;
        }
        else
        {
            // printf("Ignore SDL event 0x%04X\n", sdl_event.type);
        }
    }

exit8:
    SDL_Quit();
exit7:
    av_packet_unref(p_packet);
exit6:
    sws_freeContext(sws_ctx); 
exit5:
    av_free(buffer);
exit4:
    av_frame_free(&p_frm_yuv);
exit3:
    av_frame_free(&p_frm_raw);
exit2:
    avcodec_free_context(p_codec_ctx);
exit1:
    avformat_close_input(&p_fmt_ctx);
exit0:
    return res;
}
