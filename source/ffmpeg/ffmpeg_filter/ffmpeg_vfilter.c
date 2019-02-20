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
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

static bool s_playing_exit = false;
static bool s_playing_pause = false;

typedef struct {
    AVFormatContext *fmt_ctx;
    AVCodecContext  *dec_ctx;
    int              video_idx;
    int              frame_rate;
}   input_ctx_t;

typedef struct {
    SDL_Window      *sdl_window; 
    SDL_Renderer    *sdl_renderer;
    SDL_Texture     *sdl_texture;
    SDL_Rect        *sdl_rect;
    AVFrame         *disp_frame;
    struct SwsContext *sws_ctx;
}   disp_ctx_t;

typedef struct {
    AVFilterContext *bufsink_ctx;
    AVFilterContext *bufsrc_ctx;
    AVFilterGraph   *filter_graph;
}   filter_ctx_t;

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

// 打开输入文件，获得AVFormatContext、AVCodecContext和stream_index
static int open_input(const char *url, input_ctx_t *ictx)
{
    int ret;
    AVCodec *dec;

    ret = avformat_open_input(&(ictx->fmt_ctx), url, NULL, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        goto exit0;
    }

    ret = avformat_find_stream_info(ictx->fmt_ctx, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        goto exit1;
    }

    /* select the video stream */
    ret = av_find_best_stream(ictx->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        goto exit1;
    }
    ictx->video_idx = ret;
    ictx->frame_rate = ictx->fmt_ctx->streams[ictx->video_idx]->avg_frame_rate.num /
                       ictx->fmt_ctx->streams[ictx->video_idx]->avg_frame_rate.den;

    /* create decoding context */
    ictx->dec_ctx = avcodec_alloc_context3(dec);
    if (ictx->dec_ctx == NULL)
    {
        ret = AVERROR(ENOMEM);
        goto exit1;
    }
    avcodec_parameters_to_context(ictx->dec_ctx, ictx->fmt_ctx->streams[ictx->video_idx]->codecpar);

    /* init the video decoder */
    ret = avcodec_open2(ictx->dec_ctx, dec, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        goto exit2;
    }

    return 0;
    
exit2:
    avcodec_free_context(&ictx->dec_ctx);
exit1:
    avformat_close_input(&ictx->fmt_ctx);
exit0:
    return ret;
}

static int close_input(input_ctx_t *ictx)
{
    avcodec_free_context(&ictx->dec_ctx);
    avformat_close_input(&ictx->fmt_ctx);

    return 0;
}

static int init_displaying(const input_ctx_t *ictx, disp_ctx_t *dctx)
{
    int ret;

    // A1. 分配AVFrame
    // A1.1 分配AVFrame结构，注意并不分配data buffer(即AVFrame.*data[])
    dctx->disp_frame = av_frame_alloc();
    if (dctx->disp_frame == NULL)
    {
        printf("av_frame_alloc() for frame for displaying failed\n");
        ret = -1;
        goto exit0;
    }

    // A1.2 为AVFrame.*data[]手工分配缓冲区，用于存储sws_scale()中目的帧视频数据
    int buf_size = 
    av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
                             ictx->dec_ctx->width, 
                             ictx->dec_ctx->height, 
                             1
                            );
    // buffer将作为p_frm_yuv的视频数据缓冲区
    uint8_t *buffer = (uint8_t *)av_malloc(buf_size);
    if (buffer == NULL)
    {
        printf("av_malloc() for buffer failed\n");
        ret = -1;
        goto exit1;
    }
    // 使用给定参数设定disp_frame->data和disp_frame->linesize
    ret = av_image_fill_arrays(dctx->disp_frame->data,      // dst data[]
                               dctx->disp_frame->linesize,  // dst linesize[]
                               buffer,                      // src buffer
                               AV_PIX_FMT_YUV420P,          // pixel format
                               ictx->dec_ctx->width,        // width
                               ictx->dec_ctx->height,       // height
                               1                            // align
                               );
    if (ret < 0)
    {
        printf("av_image_fill_arrays() failed %d\n", ret);
        ret = -1;
        goto exit2;
    }

    // A2. 初始化SWS context，用于后续图像转换
    //     此处第6个参数使用的是FFmpeg中的像素格式，对比参考注释B4
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    //     如果解码后得到图像的不被SDL支持，不进行图像转换的话，SDL是无法正常显示图像的
    //     如果解码后得到图像的能被SDL支持，则不必进行图像转换
    //     这里为了编码简便，统一转换为SDL支持的格式AV_PIX_FMT_YUV420P==>SDL_PIXELFORMAT_IYUV
    dctx->sws_ctx = 
    sws_getContext(ictx->dec_ctx->width,    // src width
                   ictx->dec_ctx->height,   // src height
                   ictx->dec_ctx->pix_fmt,  // src format
                   ictx->dec_ctx->width,    // dst width
                   ictx->dec_ctx->height,   // dst height
                   AV_PIX_FMT_YUV420P,      // dst format
                   SWS_BICUBIC,             // flags
                   NULL,                    // src filter
                   NULL,                    // dst filter
                   NULL                     // param
                  );
    if (dctx->sws_ctx == NULL)
    {
        printf("sws_getContext() failed\n");
        ret = -1;
        goto exit3;
    }

    // B1. 初始化SDL子系统：缺省(事件处理、文件IO、线程)、视频、音频、定时器
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
    {  
        printf("SDL_Init() failed: %s\n", SDL_GetError()); 
        ret = -1;
        goto exit3;
    }

    // B2. 创建SDL窗口，SDL 2.0支持多窗口
    //     SDL_Window即运行程序后弹出的视频窗口，同SDL 1.x中的SDL_Surface
    dctx->sdl_window = 
    SDL_CreateWindow("simple ffplayer with video filter", 
                     SDL_WINDOWPOS_UNDEFINED,// 不关心窗口X坐标
                     SDL_WINDOWPOS_UNDEFINED,// 不关心窗口Y坐标
                     ictx->dec_ctx->width, 
                     ictx->dec_ctx->height,
                     SDL_WINDOW_OPENGL
                    );

    if (dctx->sdl_window == NULL)
    {  
        printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());  
        ret = -1;
        goto exit4;
    }

    // B3. 创建SDL_Renderer
    //     SDL_Renderer：渲染器
    dctx->sdl_renderer = SDL_CreateRenderer(dctx->sdl_window, -1, 0);
    if (dctx->sdl_renderer == NULL)
    {  
        printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());  
        ret = -1;
        goto exit4;
    }

    // B4. 创建SDL_Texture
    //     一个SDL_Texture对应一帧YUV数据，同SDL 1.x中的SDL_Overlay
    //     此处第2个参数使用的是SDL中的像素格式，对比参考注释A7
    //     FFmpeg中的像素格式AV_PIX_FMT_YUV420P对应SDL中的像素格式SDL_PIXELFORMAT_IYUV
    dctx->sdl_texture = 
    SDL_CreateTexture(dctx->sdl_renderer, 
                      SDL_PIXELFORMAT_IYUV, 
                      SDL_TEXTUREACCESS_STREAMING,
                      ictx->dec_ctx->width,
                      ictx->dec_ctx->height
                     );
    if (dctx->sdl_texture == NULL)
    {  
        printf("SDL_CreateTexture() failed: %s\n", SDL_GetError());  
        ret = -1;
        goto exit4;
    }

    // B5. 构造SDL_Rect
    dctx->sdl_rect->x = 0;
    dctx->sdl_rect->y = 0;
    dctx->sdl_rect->w = ictx->dec_ctx->width;
    dctx->sdl_rect->h = ictx->dec_ctx->height;

    return 0;

exit4:
    SDL_Quit();
exit3:
    sws_freeContext(dctx->sws_ctx); 
exit2:
    av_free(buffer);
exit1:
    av_frame_free(&dctx->disp_frame);
exit0:
    return ret;
}

static deinit_displaying(disp_ctx_t *dctx)
{
    av_frame_free(&dctx->disp_frame);
    sws_freeContext(dctx->sws_ctx);
    SDL_Quit();

    return 0;
}

static int init_filters(const char *filters_descr, const input_ctx_t *ictx, filter_ctx_t *fctx)
{
    char args[512];
    int ret = 0;
    // "buffer"滤镜：缓冲视频帧，作为滤镜图的输入
    const AVFilter *bufsrc  = avfilter_get_by_name("buffer");
    // "buffersink"滤镜：缓冲视频帧，作为滤镜图的输出
    const AVFilter *bufsink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = ictx->fmt_ctx->streams[ictx->video_idx]->time_base;
    //enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };

    // 分配一个滤镜图filter_graph
    fctx->filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !fctx->filter_graph)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            ictx->dec_ctx->width, ictx->dec_ctx->height, ictx->dec_ctx->pix_fmt,
            time_base.num, time_base.den,
            ictx->dec_ctx->sample_aspect_ratio.num, ictx->dec_ctx->sample_aspect_ratio.den);
    // 根据滤镜buffersrc、args、NULL三个参数创建滤镜实例buffersrc_ctx
    // 这个新创建的滤镜实例buffersrc_ctx命名为"in"
    // 将新创建的滤镜实例添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsrc_ctx, bufsrc, "in",
                                       args, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    // 根据滤镜buffersink、NULL、NULL三个参数创建滤镜实例buffersink_ctx
    // 这个新创建的滤镜实例buffersink_ctx命名为"out"
    // 将新创建的滤镜实例添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsink_ctx, bufsink, "out",
                                       NULL, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

#if 0
    // 设置输出像素格式为
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }
#endif

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */
    // 设置滤镜图的端点，包含此端点的滤镜图将会被连接到filters_descr
    // 描述的滤镜图中

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    // outputs变量意指buffersrc_ctx滤镜的输出引脚(output pad)
    // src缓冲区(buffersrc_ctx滤镜)的输出必须连到filters_descr中第一个
    // 滤镜的输入；filters_descr中第一个滤镜的输入标号未指定，故默认为
    // "in"，此处将buffersrc_ctx的输出标号也设为"in"，就实现了同标号相连
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = fctx->bufsrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    // inputs变量意指buffersink_ctx滤镜的输入引脚(input pad)
    // sink缓冲区(buffersink_ctx滤镜)的输入必须连到filters_descr中最后
    // 一个滤镜的输出；filters_descr中最后一个滤镜的输出标号未指定，故
    // 默认为"out"，此处将buffersink_ctx的输出标号也设为"out"，就实现了
    // 同标号相连
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = fctx->bufsink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    // 将filters_descr描述的滤镜图添加到filter_graph滤镜图中
    // 调用前：filter_graph包含两个滤镜buffersrc_ctx和buffersink_ctx
    // 调用后：filters_descr描述的滤镜图插入到filter_graph中，buffersrc_ctx连接到filters_descr
    //         的输入，filters_descr的输出连接到buffersink_ctx，filters_descr只进行了解析而不
    //         建立内部滤镜间的连接。filters_desc与filter_graph间的连接是利用AVFilterInOut inputs
    //         和AVFilterInOut inputs连接起来的，AVFilterInOut是一个链表，最终可用的连在一起的
    //         滤镜链/滤镜图就是通过这个链表串在一起的。
    ret = avfilter_graph_parse_ptr(fctx->filter_graph, filters_descr,
                                   &inputs, &outputs, NULL);
    if (ret < 0)
    {
        goto end;
    }

    // 验证有效性并配置filtergraph中所有连接和格式
    ret = avfilter_graph_config(fctx->filter_graph, NULL);
    if (ret < 0)
    {
        goto end;
    }

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

static int deinit_filters(filter_ctx_t *fctx)
{
    avfilter_graph_free(&fctx->filter_graph);

    return 0;
}

// return -1: error, 0: need more packet, 1: success
static int decode_video_frame(AVCodecContext *dec_ctx, AVPacket *packet, AVFrame *frame)
{
    int ret = avcodec_send_packet(dec_ctx, packet);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
        return ret;
    }

    ret = avcodec_receive_frame(dec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return 0;
    }
    else if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
        return ret;
    }

    frame->pts = frame->best_effort_timestamp;

    return 1;
}

static int filtering_video_frame(const filter_ctx_t *fctx, AVFrame *frame_in, AVFrame *frame_out)
{
    int ret;
    
    // 将frame送入filtergraph
    ret = av_buffersrc_add_frame_flags(fctx->bufsrc_ctx, frame_in, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return ret;
    }
    
    // 从filtergraph获取经过处理的frame
    ret = av_buffersink_get_frame(fctx->bufsink_ctx, frame_out);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        av_log(NULL, AV_LOG_WARNING, "Need more frames\n");
        return 0;
    }
    else if (ret < 0)
    {
        return ret;
    }

    return 1;
}

static int display_video_frame(const AVFrame *frame, disp_ctx_t *dctx)
{
    // A1. 图像转换：p_frm_raw->data ==> p_frm_yuv->data
    // 将源图像中一片连续的区域经过处理后更新到目标图像对应区域，处理的图像区域必须逐行连续
    // plane: 如YUV有Y、U、V三个plane，RGB有R、G、B三个plane
    // slice: 图像中一片连续的行，必须是连续的，顺序由顶部到底部或由底部到顶部
    // stride/pitch: 一行图像所占的字节数，Stride=BytesPerPixel*Width+Padding，注意对齐
    // AVFrame.*data[]: 每个数组元素指向对应plane
    // AVFrame.linesize[]: 每个数组元素表示对应plane中一行图像所占的字节数
    sws_scale(dctx->sws_ctx,                            // sws context
              (const uint8_t *const *)frame->data,      // src slice
              frame->linesize,                          // src stride
              0,                                        // src slice y
              frame->height,                            // src slice height
              dctx->disp_frame->data,                   // dst planes
              dctx->disp_frame->linesize                // dst strides
              );
    
    // B7. 使用新的YUV像素数据更新SDL_Rect
    SDL_UpdateYUVTexture(dctx->sdl_texture,             // sdl texture
                         dctx->sdl_rect,                // sdl rect
                         dctx->disp_frame->data[0],     // y plane
                         dctx->disp_frame->linesize[0], // y pitch
                         dctx->disp_frame->data[1],     // u plane
                         dctx->disp_frame->linesize[1], // u pitch
                         dctx->disp_frame->data[2],     // v plane
                         dctx->disp_frame->linesize[2]  // v pitch
                         );
    
    // B8. 使用特定颜色清空当前渲染目标
    SDL_RenderClear(dctx->sdl_renderer);
    // B9. 使用部分图像数据(texture)更新当前渲染目标
    SDL_RenderCopy(dctx->sdl_renderer,                  // sdl renderer
                   dctx->sdl_texture,                   // sdl texture
                   NULL,                                // src rect, if NULL copy texture
                   dctx->sdl_rect                       // dst rect
                   );
    
    // B10. 执行渲染，更新屏幕显示
    SDL_RenderPresent(dctx->sdl_renderer);

}

// test as:
// ./ffplayer testsrc -vf transpose=cclock
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s [file] -vf [filtergraph]\n", argv[0]);
        exit(1);
    }

    int ret;
    input_ctx_t in_ctx = { NULL, NULL, -1, 0};
    ret = open_input(argv[1], &in_ctx);
    if (ret < 0)
    {
        goto exit0;
    }

    filter_ctx_t filter_ctx = { NULL, NULL, NULL };
    ret = init_filters(argv[3], &in_ctx, &filter_ctx);
    if (ret < 0)
    {
        goto exit1;
    }

    SDL_Rect sdl_rect = {0, 0, 0, 0};
    disp_ctx_t disp_ctx = {NULL, NULL, NULL, &sdl_rect, NULL, NULL};
    ret = init_displaying(&in_ctx, &disp_ctx);
    if (ret < 0)
    {
        goto exit2;
    }

    AVPacket packet;
    AVFrame *frame = NULL;
    AVFrame *filt_frame = NULL;
    AVFrame *p_frame = NULL;
    frame = av_frame_alloc();
    filt_frame = av_frame_alloc();
    if (!frame || !filt_frame)
    {
        perror("Could not allocate frame");
        goto exit3;
    }

    // B5. 创建定时刷新事件线程，按照预设帧率产生刷新事件
    SDL_Thread* sdl_thread = 
    SDL_CreateThread(sdl_thread_handle_refreshing, NULL, (void *)&in_ctx.frame_rate);
    if (sdl_thread == NULL)
    {  
        printf("SDL_CreateThread() failed: %s\n", SDL_GetError());  
        ret = -1;
        goto exit4;
    }

    SDL_Event sdl_event;
    while (1)
    {
        SDL_WaitEvent(&sdl_event);
        if (sdl_event.type == SDL_USEREVENT_REFRESH)
        {
            continue;
        }
        
        // 读取packet
        ret = av_read_frame(in_ctx.fmt_ctx, &packet);
        if (ret < 0)
        {
            goto exit4;
        }

        if (packet.stream_index != in_ctx.video_idx)
        {
            continue;
        }

        ret = decode_video_frame(in_ctx.dec_ctx, &packet, frame);
        if (ret < 0)
        {
            goto exit4;
        }

        ret = filtering_video_frame(&filter_ctx, frame, filt_frame);
        if (ret < 0)
        {
            p_frame = frame;
        }
        else
        {
            p_frame = filt_frame;
        }

        ret = display_video_frame(p_frame, &disp_ctx);
        if (ret < 0)
        {
            goto exit4;
        }
        
        av_frame_unref(filt_frame);
        av_frame_unref(frame);
        av_packet_unref(&packet);
    }

exit4:
    av_frame_unref(filt_frame);
    av_frame_unref(frame);
exit3:
    deinit_displaying(&disp_ctx);
exit2:
    deinit_filters(&filter_ctx);
exit1:
    close_input(&in_ctx);;
exit0:
    return ret;
}

