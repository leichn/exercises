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

int init_filters(const char *filters_descr, filter_ctx_t *fctx);
int deinit_filters(filter_ctx_t *fctx);
int filtering_video_frame(const filter_ctx_t *fctx, AVFrame *frame_in, AVFrame *frame_out);

#endif
