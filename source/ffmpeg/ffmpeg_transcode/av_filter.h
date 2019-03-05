#ifndef __AV_FILTER_H__
#define __AV_FILTER_H__

#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "open_file.h"

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
    int sample_rate;
    uint64_t channel_layout;
    int nb_channels;
    AVRational time_base;
}   filter_iafmt_t;

typedef struct {
    enum AVSampleFormat *sample_fmts;
    int *sample_rates;
    uint64_t *channel_layouts;
}   filter_oafmt_t;

int init_video_filters(const char *filters_descr, const filter_ivfmt_t *ivfmt, 
                       const filter_ovfmt_t *ovfmt, filter_ctx_t *fctx);
int init_audio_filters(const char *filters_descr, const filter_iafmt_t *ivfmt, 
                       const filter_oafmt_t *ovfmt, filter_ctx_t *fctx);
int deinit_filters(filter_ctx_t *fctx);
void get_filter_ivfmt(const inout_ctx_t *ictx, int stream_idx, filter_ivfmt_t *ivfmt);
void get_filter_iafmt(const inout_ctx_t *ictx, int stream_idx, filter_iafmt_t *iafmt);
int filtering_frame(const filter_ctx_t *fctx, AVFrame *frame_in, AVFrame *frame_out);

#endif

