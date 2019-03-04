#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libavutil/channel_layout.h>
#include "av_filter.h"

void get_filter_ivfmt(const inout_ctx_t *ictx, int stream_idx, filter_ivfmt_t *ivfmt)
{
    ivfmt->width = ictx->codec_ctx[stream_idx]->width;
    ivfmt->height = ictx->codec_ctx[stream_idx]->height;
    ivfmt->pix_fmt = ictx->codec_ctx[stream_idx]->pix_fmt;
    ivfmt->sar = ictx->codec_ctx[stream_idx]->sample_aspect_ratio;
    ivfmt->time_base = ictx->fmt_ctx->streams[stream_idx]->time_base;
    ivfmt->frame_rate = ictx->fmt_ctx->streams[stream_idx]->avg_frame_rate;
    av_log(NULL, AV_LOG_INFO, "get video format: "
            "%dx%d, pix_fmt %d, SAR %d/%d, tb {%d, %d}, rate {%d, %d}\n",
            ivfmt->width, ivfmt->height, ivfmt->pix_fmt,
            ivfmt->sar.num, ivfmt->sar.den,
            ivfmt->time_base.num, ivfmt->time_base.den,
            ivfmt->frame_rate.num, ivfmt->frame_rate.den);
}

void get_filter_ovfmt(const inout_ctx_t *octx, int stream_idx, filter_ovfmt_t *ovfmt)
{
}

void get_filter_iafmt(const inout_ctx_t *ictx, int stream_idx, filter_iafmt_t *iafmt)
{
    iafmt->sample_fmt = ictx->codec_ctx[stream_idx]->sample_fmt;
    iafmt->sample_rate = ictx->codec_ctx[stream_idx]->sample_rate;
    iafmt->nb_channels = ictx->codec_ctx[stream_idx]->channels;
    iafmt->channel_layout = ictx->codec_ctx[stream_idx]->channel_layout;
    iafmt->time_base = ictx->codec_ctx[stream_idx]->time_base;
}

void get_filter_oafmt(const inout_ctx_t *octx, int stream_idx, filter_oafmt_t *oafmt)
{
}


// 创建配置一个滤镜图，在后续滤镜处理中，可以往此滤镜图输入数据并从滤镜图获得输出数据
// @filters_descr: I, 以字符串形式描述的滤镜图，形如"transpose=cclock,pad=iw*2:ih"
// @vfmt:          I, 输入图像格式，用于设置滤镜图输入节点(buffer滤镜)
// @pix_fmts:      I, 输出像素格式，用于设置滤镜图输出节点(buffersink滤镜)。
//                    数组形式，以-1标识有效元素结束，形如{AV_PIX_FMT_YUV420P, AV_PIX_FMT_RGB24, -1}
// @fctx:          O, 配置好的 filter context
int init_video_filters(const char *filters_descr, 
                       const filter_ivfmt_t *ivfmt, 
                       const filter_ovfmt_t *ovfmt,
                       filter_ctx_t *fctx)
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
    char *p_args = NULL;
    if (ivfmt != NULL)
    {
        /* buffer video source: the decoded frames from the decoder will be inserted here. */
        // args是buffersrc滤镜的参数
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 ivfmt->width, ivfmt->height, ivfmt->pix_fmt, 
                 ivfmt->time_base.num, ivfmt->time_base.den, 
                 ivfmt->sar.num, ivfmt->sar.den);
        p_args = args;
    }

    // buffer滤镜：缓冲视频帧，作为滤镜图的输入
    const AVFilter *bufsrc  = avfilter_get_by_name("buffer");
    // 为buffersrc滤镜创建滤镜实例buffersrc_ctx，命名为"in"
    // 将新创建的滤镜实例buffersrc_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsrc_ctx, bufsrc, "in",
                                       p_args, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    // buffersink滤镜：缓冲视频帧，作为滤镜图的输出
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

    // 设置输出像素格式为pix_fmts[]中指定的格式(如果要用SDL显示，则这些格式应是SDL支持格式)
    ret = av_opt_set_int_list(fctx->bufsink_ctx, "pix_fmts", ovfmt->pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */
    // 设置滤镜图的端点，将filters_descr描述的滤镜图连接到此滤镜图，
    // 两个滤镜图的连接是通过端点连接(AVFilterInOut)完成的

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
    AVFilterInOut *outputs = avfilter_inout_alloc();
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
    AVFilterInOut *inputs  = avfilter_inout_alloc();
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

// 创建配置一个滤镜图，在后续滤镜处理中，可以往此滤镜图输入数据并从滤镜图获得输出数据
// @filters_descr: I, 以字符串形式描述的滤镜图，形如""
// @afmt:          I, 输入声音格式，用于设置滤镜图输入节点(abuffer滤镜)
// @pix_fmts:      I, 输出像素格式，用于设置滤镜图输出节点(abuffersink滤镜)。
//                    数组形式，以-1标识有效元素结束，形如{AV_PIX_FMT_YUV420P, AV_PIX_FMT_RGB24, -1}
// @fctx:          O, 配置好的 filter context
int init_audio_filters(const char *filters_descr, 
                       const filter_iafmt_t *iafmt, 
                       const filter_oafmt_t *oafmt, 
                       filter_ctx_t *fctx)
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
    char *p_args = NULL;
    if (iafmt != NULL)
    {
        uint64_t channel_layout = iafmt->channel_layout > 0 ? iafmt->channel_layout :
                                  av_get_default_channel_layout(iafmt->nb_channels);
        // args是abuffersrc滤镜的参数
        snprintf(args, sizeof(args),
                "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
                iafmt->time_base.num, iafmt->time_base.den, iafmt->sample_rate,
                av_get_sample_fmt_name(iafmt->sample_fmt), channel_layout);
        p_args = args;
    }

    // buffer滤镜：缓冲视频帧，作为滤镜图的输入
    const AVFilter *bufsrc  = avfilter_get_by_name("abuffer");
    // 为buffersrc滤镜创建滤镜实例buffersrc_ctx，命名为"in"
    // 将新创建的滤镜实例buffersrc_ctx添加到滤镜图filter_graph中
    ret = avfilter_graph_create_filter(&fctx->bufsrc_ctx, bufsrc, "in",
                                       p_args, NULL, fctx->filter_graph);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    // buffersink滤镜：缓冲视频帧，作为滤镜图的输出
    const AVFilter *bufsink = avfilter_get_by_name("abuffersink");
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


    ret = av_opt_set_int_list(fctx->bufsink_ctx, "sample_fmts",
            oafmt->sample_fmts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
        goto end;
    }
    
    // 将输出声道布局设置为编码器采用的声道布局
    ret = av_opt_set_int_list(fctx->bufsink_ctx, "channel_layouts",
            oafmt->channel_layouts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
        goto end;
    }
    
    ret = av_opt_set_int_list(fctx->bufsink_ctx, "sample_rates",
            oafmt->sample_rates, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */
    // 设置滤镜图的端点，将filters_descr描述的滤镜图连接到此滤镜图，
    // 两个滤镜图的连接是通过端点连接(AVFilterInOut)完成的

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
    AVFilterInOut *outputs = avfilter_inout_alloc();
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
    AVFilterInOut *inputs  = avfilter_inout_alloc();
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

int deinit_filters(filter_ctx_t *fctx)
{
    avfilter_graph_free(&fctx->filter_graph);

    return 0;
}

int filtering_frame(const filter_ctx_t *fctx, AVFrame *frame_in, AVFrame *frame_out)
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



