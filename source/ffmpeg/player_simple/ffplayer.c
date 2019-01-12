/*****************************************************************
 * ffplayer.c
 *
 * history:
 *   2018-11-27 - [lei]     created file
 *
 * details:
 *   A simple ffmpeg player.
 *
 * refrence:
 *   1. https://blog.csdn.net/leixiaohua1020/article/details/38868499
 *   2. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial01.html
 *   3. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial02.html
******************************************************************/

#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

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
    SDL_Window*         screen; 
    SDL_Renderer*       sdl_renderer;
    SDL_Texture*        sdl_texture;
    SDL_Rect            sdl_rect;

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
        printf("avformat_open_input() failed\n");
        return -1;
    }

    // A2. 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入pFormatCtx->streams
    //     p_fmt_ctx->streams是一个指针数组，数组大小是pFormatCtx->nb_streams
    ret = avformat_find_stream_info(p_fmt_ctx, NULL);
    if (ret < 0)
    {
        printf("avformat_find_stream_info() failed\n");
        return -1;
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
            break;
        }
    }
    if (v_idx == -1)
    {
        printf("Cann't find a video stream\n");
        return -1;
    }

    // A5. 为视频流构建解码器AVCodecContext

    // A5.1 获取解码器参数AVCodecParameters
    p_codec_par = p_fmt_ctx->streams[v_idx]->codecpar;
    // A5.2 获取解码器
    p_codec = avcodec_find_decoder(p_codec_par->codec_id);
    if (p_codec == NULL)
    {
        printf("Cann't find codec!\n");
        return -1;
    }
    // A5.3 构建解码器AVCodecContext
    // A5.3.1 p_codec_ctx初始化：分配结构体，使用p_codec初始化相应成员为默认值
    p_codec_ctx = avcodec_alloc_context3(p_codec);

    // A5.3.2 p_codec_ctx初始化：p_codec_par ==> p_codec_ctx，初始化相应成员
    ret = avcodec_parameters_to_context(p_codec_ctx, p_codec_par);
    if (ret < 0)
    {
        printf("avcodec_parameters_to_context() failed %d\n", ret);
        return -1;
    }

    // A5.3.3 p_codec_ctx初始化：使用p_codec初始化p_codec_ctx，初始化完成
    ret = avcodec_open2(p_codec_ctx, p_codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2() failed %d\n", ret);
        return -1;
    }

    // A6. 分配AVFrame
    // A6.1 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
    p_frm_raw = av_frame_alloc();
    p_frm_yuv = av_frame_alloc();

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
    // 使用给定参数设定p_frm_yuv->data和p_frm_yuv->linesize
    av_image_fill_arrays(p_frm_yuv->data,           // dst data[]
                         p_frm_yuv->linesize,       // dst linesize[]
                         buffer,                    // src buffer
                         AV_PIX_FMT_YUV420P,        // pixel format
                         p_codec_ctx->width,        // width
                         p_codec_ctx->height,       // height
                         1                          // align
                        );

    // A7. 初始化SWS context，用于后续图像转换
    //     此处第6个参数使用的是FFmpeg中的像素格式，对比参考注释B4
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    //     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
    //     如果解码后得到图像的能被SDL支持，则不必进行图像转换
    //     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
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


    // B1. 初始化SDL子系统：缺省(事件处理、文件IO、线程)、视频、音频、定时器
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {  
        printf("SDL_Init() failed: %s\n", SDL_GetError()); 
        return -1;
    }
    
    // B2. 创建SDL窗口，SDL 2.0支持多窗口
    //     SDL_Window即运行程序后弹出的视频窗口，同SDL 1.x中的SDL_Surface
    screen = SDL_CreateWindow("Simplest ffmpeg player's Window", 
                              SDL_WINDOWPOS_UNDEFINED,// 不关心窗口X坐标
                              SDL_WINDOWPOS_UNDEFINED,// 不关心窗口Y坐标
                              p_codec_ctx->width, 
                              p_codec_ctx->height,
                              SDL_WINDOW_OPENGL
                             );

    if (screen == NULL)
    {  
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());  
        return -1;
    }

    // B3. 创建SDL_Renderer
    //     SDL_Renderer：渲染器
    sdl_renderer = SDL_CreateRenderer(screen, -1, 0);

    // B4. 创建SDL_Texture
    //     一个SDL_Texture对应一帧YUV数据，同SDL 1.x中的SDL_Overlay
    //     此处第2个参数使用的是SDL中的像素格式，对比参考注释A7
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    sdl_texture = SDL_CreateTexture(sdl_renderer, 
                                    SDL_PIXELFORMAT_IYUV, 
                                    SDL_TEXTUREACCESS_STREAMING,
                                    p_codec_ctx->width,
                                    p_codec_ctx->height);  

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = p_codec_ctx->width;
    sdl_rect.h = p_codec_ctx->height;

    p_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    // A8. 从视频文件中读取一个packet
    //     packet可能是视频帧、音频帧或其他数据，解码器只会解码视频帧或音频帧，非音视频数据并不会被
    //     扔掉、从而能向解码器提供尽可能多的信息
    //     对于视频来说，一个packet只包含一个frame
    //     对于音频来说，若是帧长固定的格式则一个packet可包含整数个frame，
    //                   若是帧长可变的格式则一个packet只包含一个frame
    while (av_read_frame(p_fmt_ctx, p_packet) == 0)
    {
        if (p_packet->stream_index == v_idx)  // 仅处理视频帧
        {
            // A9. 视频解码：packet ==> frame
            // A9.1 向解码器喂数据，一个packet可能是一个视频帧或多个音频帧，此处音频帧已被上一句滤掉
            ret = avcodec_send_packet(p_codec_ctx, p_packet);
            if (ret != 0)
            {
                printf("avcodec_send_packet() failed %d\n", ret);
                return -1;
            }
            // A9.2 接收解码器输出的数据，此处只处理视频帧，每次接收一个packet，将之解码得到一个frame
            ret = avcodec_receive_frame(p_codec_ctx, p_frm_raw);
            if (ret != 0)
            {
                printf("avcodec_receive_frame() failed %d\n", ret);
                return -1;
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
            // B7. 使用部分图像数据(texture)更新当前渲染目标
            SDL_RenderCopy(sdl_renderer,                        // sdl renderer
                           sdl_texture,                         // sdl texture
                           NULL,                                // src rect, if NULL copy texture
                           &sdl_rect                            // dst rect
                          );
            // B8. 执行渲染，更新屏幕显示
            SDL_RenderPresent(sdl_renderer);  

            // B9. 控制帧率为25FPS，此处不够准确，未考虑解码消耗的时间
            SDL_Delay(40);
        }
        av_packet_unref(p_packet);
    }

    SDL_Quit();
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&p_frm_yuv);
    av_frame_free(&p_frm_raw);
    avcodec_close(p_codec_ctx);
    avformat_close_input(&p_fmt_ctx);

    return 0;
}
