#ifndef __VIDEO_FILTER_H__
#define __VIDEO_FILTER_H__

#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/frame.h>

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
}   input_vfmt_t;


int init_video_filters(const char *filters_descr, const input_vfmt_t *vfmt, filter_ctx_t *fctx);
int deinit_filters(filter_ctx_t *fctx);
int filtering_video_frame(const filter_ctx_t *fctx, AVFrame *frame_in, AVFrame *frame_out);

#endif
