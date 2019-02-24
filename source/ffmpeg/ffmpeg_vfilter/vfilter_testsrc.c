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
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "video_play.h"
#include "video_filter.h"

#include <SDL2/SDL.h>

#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

// 通过interval参数传入当前的timer interval，返回下一次timer的interval，返回0表示取消定时器
// 定时器超时时间到时调用此回调函数，产生FF_REFRESH_EVENT事件，添加到事件队列
static uint32_t sdl_time_cb_refresh(uint32_t interval, void *opaque)
{
    SDL_Event sdl_event;
    sdl_event.type = SDL_USEREVENT_REFRESH;
    SDL_PushEvent(&sdl_event);  // 将事件添加到事件队列，此队列可读可写
    return interval;            // 返回0表示停止定时器 
}


#if 0   // TODO: 如何使用API探测得到"-f lavfi -i testsrc"格式
// 探测视频格式
static int probe_format(const char *url, input_vfmt_t *vfmt)
{
    int ret;

    AVFormatContext *fmt_ctx = NULL;
    AVInputFormat *ifmt = av_find_input_format("lavfi");
    if (ifmt == NULL)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find input format lavfi\n");
        goto exit0;
    }
    av_log(NULL, AV_LOG_INFO, "input format: %s, %s, %s\n", ifmt->name, ifmt->long_name, ifmt->extensions);
    ret = avformat_open_input(&fmt_ctx, NULL, ifmt, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        goto exit0;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        goto exit1;
    }

    /* select the video stream */
    AVCodec *dec = NULL;
    int v_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (v_idx < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        goto exit1;
    }

    /* create decoding context */
    AVCodecContext  *dec_ctx = avcodec_alloc_context3(dec);
    if (dec_ctx == NULL)
    {
        ret = AVERROR(ENOMEM);
        goto exit1;
    }
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[v_idx]->codecpar);

    /* init the video decoder */
    ret = avcodec_open2(dec_ctx, dec, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        goto exit2;
    }

    vfmt->width = dec_ctx->width;
    vfmt->height = dec_ctx->height;
    vfmt->pix_fmt = dec_ctx->pix_fmt;
    vfmt->sar = dec_ctx->sample_aspect_ratio;
    vfmt->time_base = fmt_ctx->streams[v_idx]->time_base;
    vfmt->frame_rate = fmt_ctx->streams[v_idx]->avg_frame_rate;
    av_log(NULL, AV_LOG_INFO, "probe video format: "
           "%dx%d, pix_fmt %d, SAR %d/%d, tb {%d, %d}, rate {%d, %d}\n",
           vfmt->width, vfmt->height, vfmt->pix_fmt,
           vfmt->sar.num, vfmt->sar.den,
           vfmt->time_base.num, vfmt->time_base.den,
           vfmt->frame_rate.num, vfmt->frame_rate.den);

    ret = 0;
    
exit2:
    avcodec_free_context(&dec_ctx);
exit1:
    avformat_close_input(&fmt_ctx);
exit0:
    return ret;
}
#endif

// @filter [i]  产生测试图案的filter
// @vfmt   [o]  @filter的参数
// @fctx   [o]  用户定义的数据类型，输出供调用者使用
static int open_testsrc(const char *filter, input_vfmt_t *vfmt, filter_ctx_t *fctx)
{
    int ret = 0;

    // 分配一个滤镜图filter_graph
    fctx->filter_graph = avfilter_graph_alloc();
    if (!fctx->filter_graph)
    {
        return AVERROR(ENOMEM);
    }

    // source滤镜：合法值有"testsrc"/"smptebars"/"color=c=blue"/...
    const AVFilter *bufsrc  = avfilter_get_by_name(filter);
    // 为buffersrc滤镜创建滤镜实例buffersrc_ctx，命名为"in"
    // 将新创建的滤镜实例buffersrc_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsrc_ctx, bufsrc, "in",
                                       NULL, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create filter testsrc\n");
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
        av_log(NULL, AV_LOG_ERROR, "Cannot create filter buffersink\n");
        goto end;
    }

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

    vfmt->pix_fmt = av_buffersink_get_format(fctx->bufsink_ctx);
    vfmt->width = av_buffersink_get_w(fctx->bufsink_ctx);
    vfmt->height = av_buffersink_get_h(fctx->bufsink_ctx);
    vfmt->sar = av_buffersink_get_sample_aspect_ratio(fctx->bufsink_ctx);
    vfmt->time_base = av_buffersink_get_time_base(fctx->bufsink_ctx);
    vfmt->frame_rate = av_buffersink_get_frame_rate(fctx->bufsink_ctx);

    av_log(NULL, AV_LOG_INFO, "probe video format: "
           "%dx%d, pix_fmt %d, SAR %d/%d, tb {%d, %d}, rate {%d, %d}\n",
           vfmt->width, vfmt->height, vfmt->pix_fmt,
           vfmt->sar.num, vfmt->sar.den,
           vfmt->time_base.num, vfmt->time_base.den,
           vfmt->frame_rate.num, vfmt->frame_rate.den);

    return 0;

end:
    avfilter_graph_free(&fctx->filter_graph);
    return ret;
}

static int close_testsrc(filter_ctx_t *fctx)
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

    input_vfmt_t input_vfmt;
    filter_ctx_t input_ctx = { NULL, NULL, NULL };
    ret = open_testsrc(argv[1], &input_vfmt, &input_ctx);
    if (ret < 0)
    {
        goto exit0;
    }

#if 0
    input_vfmt_t input_vfmt;
    ret = probe_format(argv[1], &input_vfmt);
    if (ret < 0)
    {
        goto exit0;
    }
#endif

    filter_ctx_t filter_ctx = { NULL, NULL, NULL };
    ret = init_filters(argv[3], &input_vfmt, &filter_ctx);
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

    int temp_num = input_vfmt.frame_rate.num;
    int temp_den = input_vfmt.frame_rate.den;
    int interval = (temp_num > 0) ? (temp_den*1000)/temp_num : 40;

    printf("frame rate %d/%d FPS, refresh interval %d ms\n", temp_num/temp_den, interval);

    // 创建视频解码定时刷新线程，此线程为SDL内部线程，调用指定的回调函数
    SDL_AddTimer(interval, sdl_time_cb_refresh, NULL);
    SDL_Event sdl_event;

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

        SDL_WaitEvent(&sdl_event);
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

