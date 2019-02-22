/*******************************************************************************
 * ffplayer.c
 *
 * history:
 *   2019-02-20 - [lei]     Create file
 *
 * details:
 *   A simple ffmpeg player with video filter.
 *******************************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#include "video_play.h"
#include "video_filter.h"

#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

static bool s_playing_exit = false;
static bool s_playing_pause = false;

typedef struct {
    AVFilterContext *bufsink_ctx;
    AVFilterContext *bufsrc_ctx;
    AVFilterGraph   *filter_graph;
}   filter_ctx_t;

typedef struct {
    int width;
    int height;
    enum AVPixelFormat pix_fmt;
    AVRational time_base;
    AVRational sar;
    AVRational frame_rate;
}   input_avopt_t;

// 探测
static int probe_testsrc_format(const char *url, input_avopt_t *avopt)
{
    int ret;


    AVInputFormat *ifmt = av_find_input_format("lavfi");
    ret = avformat_open_input(&(ictx->fmt_ctx), url, ifmt, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        goto exit0;
    }

    AVFormatContext *fmt_ctx;
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        goto exit1;
    }

    /* select the video stream */
    AVCodec *dec;
    int v_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (v_idx < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        goto exit1;
    }
    ictx->frame_rate = fmt_ctx->streams[v_idx]->avg_frame_rate.num /
                       fmt_ctx->streams[v_idx]->avg_frame_rate.den;

    /* create decoding context */
    AVCodecContext  *dec_ctx = avcodec_alloc_context3(dec);
    if (dec_ctx == NULL)
    {
        ret = AVERROR(ENOMEM);
        goto exit1;
    }
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[ictx->video_idx]->codecpar);

    /* init the video decoder */
    ret = avcodec_open2(dec_ctx, dec, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        goto exit2;
    }

    avopt->width = dec_ctx->width;
    avopt->height = dec_ctx->height;
    avopt->pix_fmt = dec_ctx->pix_fmt;
    avopt->sar = dec_ctx->sample_aspect_ratio;
    avopt->time_base = fmt_ctx->streams[v_idx]->time_base;
    avopt->frame_rate

    return 0;
    
exit2:
    avcodec_free_context(&dec_ctx);
exit1:
    avformat_close_input(&fmt_ctx);
exit0:
    return ret;
}


static int open_testsrc(filter_ctx_t *fctx)
{
    char args[512];
    int ret = 0;
    // "testsrc"滤镜：
    const AVFilter *bufsrc  = avfilter_get_by_name("testsrc");
    // "buffersink"滤镜：缓冲视频帧，作为滤镜图的输出
    const AVFilter *bufsink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();

    // 分配一个滤镜图filter_graph
    fctx->filter_graph = avfilter_graph_alloc();
    if (!fctx->filter_graph)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    // 为buffersrc滤镜创建滤镜实例buffersrc_ctx，命名为"in"
    // 将新创建的滤镜实例buffersrc_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsrc_ctx, bufsrc, "in",
                                       NULL, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create filter testsrc\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    // 为buffersink滤镜创建滤镜实例buffersink_ctx，命名为"out"
    // 将新创建的滤镜实例buffersink_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsink_ctx, bufsink, "out",
                                       NULL, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create filter buffersink\n");
        goto end;
    }

#if 0   // 因为后面显示视频帧时有sws_scale()进行图像格式转换，帮此处不设置滤镜输出格式也可
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUYV422, AV_PIX_FMT_NONE };
    // 设置输出像素格式为pix_fmts[]中指定的格式(如果要用SDL显示，则这些格式应是SDL支持格式)
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }
#endif

    if ((ret = avfilter_link(fctx->bufsrc_ctx, 0, fctx->bufsink_ctx, 0)) < 0)
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

static int close_testsrc(filter_ctx_t *fctx)
{
    avfilter_graph_free(&fctx->filter_graph);

    return 0;
}

static int init_filters(const char *filters_descr, filter_ctx_t *fctx)
{
    int ret = 0;

    // 分配一个滤镜图filter_graph
    fctx->filter_graph = avfilter_graph_alloc();
    if (!fctx->filter_graph)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    char args[512];
    int w = 320;
    int h = 240;
    AVRational time_base = {1, 25};
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    // args是buffersrc滤镜的参数
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             w, h, AV_PIX_FMT_RGB24, time_base.num, time_base.den, 1, 1);

    // "buffer"滤镜：缓冲视频帧，作为滤镜图的输入
    const AVFilter *bufsrc  = avfilter_get_by_name("buffer");
    // 为buffersrc滤镜创建滤镜实例buffersrc_ctx，命名为"in"
    // 将新创建的滤镜实例buffersrc_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsrc_ctx, bufsrc, "in",
                                       args, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    // "buffersink"滤镜：缓冲视频帧，作为滤镜图的输出
    const AVFilter *bufsink = avfilter_get_by_name("buffersink");
    /* buffer video sink: to terminate the filter chain. */
    // 为buffersink滤镜创建滤镜实例buffersink_ctx，命名为"out"
    // 将新创建的滤镜实例buffersink_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsink_ctx, bufsink, "out",
                                       NULL, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

#if 0   // 因为后面显示视频帧时有sws_scale()进行图像格式转换，帮此处不设置滤镜输出格式也可
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUYV422, AV_PIX_FMT_NONE };
    // 设置输出像素格式为pix_fmts[]中指定的格式(如果要用SDL显示，则这些格式应是SDL支持格式)
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

    AVFilterInOut *outputs = avfilter_inout_alloc();

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

    AVFilterInOut *inputs  = avfilter_inout_alloc();

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
    //         和AVFilterInOut outputs连接起来的，AVFilterInOut是一个链表，最终可用的连在一起的
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

static int read_testsrc_frame(const filter_ctx_t *fctx, AVFrame *frame_out)
{
    int ret;
    
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

// test as:
// ./vfilter testsrc -vf transpose=cclock,pad=iw+80:ih:40 
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s testsrc -vf [filtergraph]\n", argv[0]);
        exit(1);
    }

    int ret = 0;

    // 初始化SDL子系统：缺省(事件处理、文件IO、线程)、视频、音频、定时器
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS))
    {  
        av_log(NULL, AV_LOG_ERROR, "SDL_Init() failed: %s\n", SDL_GetError()); 
        ret = -1;
        goto exit0;
    }

    filter_ctx_t input_ctx = { NULL, NULL, NULL };
    ret = open_testsrc(&input_ctx);
    if (ret < 0)
    {
        goto exit0;
    }

    filter_ctx_t filter_ctx = { NULL, NULL, NULL };
    ret = init_filters(argv[3], &filter_ctx);
    if (ret < 0)
    {
        goto exit1;
    }

    disp_ctx_t disp_ctx = {NULL, NULL, NULL, NULL, false};
    ret = init_displaying(&disp_ctx);
    if (ret < 0)
    {
        goto exit2;
    }

    AVFrame *frame = NULL;
    AVFrame *filt_frame = NULL;
    frame = av_frame_alloc();
    filt_frame = av_frame_alloc();
    if (!frame || !filt_frame)
    {
        perror("Could not allocate frame");
        goto exit3;
    }

    while (1)
    {
        ret = read_testsrc_frame(&input_ctx, frame);
        if (ret == 0)
        {
            continue;
        }
        else if (ret < 0)
        {
            goto exit4;
        }

        ret = filtering_video_frame(&filter_ctx, frame, filt_frame);
        if (ret == 0)
        {
            continue;
        }
        if (ret < 0)
        {
            goto exit4;
        }

        play_video_frame(&disp_ctx, filt_frame);
        
        av_frame_unref(filt_frame);
        av_frame_unref(frame);
    }

exit4:
    av_frame_unref(filt_frame);
    av_frame_unref(frame);
exit3:
    deinit_displaying(&disp_ctx);
exit2:
    deinit_filters(&filter_ctx);
exit1:
    close_testsrc(&input_ctx);
exit0:
    return ret;
}

