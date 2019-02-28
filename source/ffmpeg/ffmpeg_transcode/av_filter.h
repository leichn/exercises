#ifndef __AV_FILTER_H__
#define __AV_FILTER_H__

#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/frame.h>

typedef struct {
    AVFilterContext *bufsink_ctx;
    AVFilterContext *bufsrc_ctx;
    AVFilterGraph   *filter_graph;
}   filter_ctx_t;

// input video format for buffer filter
typedef struct {
    int width;
    int height;
    enum AVPixelFormat pix_fmt;
    AVRational time_base;
    AVRational sar;
    AVRational frame_rate;
}   filter_ivfmt_t;

// output video format for buffersink filter
typedef struct {
    enum AVPixelFormat *pix_fmts;
}   filter_ovfmt_t;

typedef struct {
    enum AVSampleFormat sample_fmt;
    AVRational time_base;
    AVRational sample_rate;
    uint64_t channel_layout;
}   filter_iafmt_t;

typedef struct {
    enum AVSampleFormat *sample_fmts;
    AVRational *sample_rates;
    uint64_t *channel_layouts;
}   filter_oafmt_t;

int init_filters(const char *filters_descr, const input_vfmt_t *vfmt, filter_ctx_t *fctx);
int deinit_filters(filter_ctx_t *fctx);
int filtering_video_frame(const filter_ctx_t *fctx, AVFrame *frame_in, AVFrame *frame_out);

#endif

