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
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "video_play.h"
#include "video_filter.h"

#include <SDL2/SDL.h>

#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

static bool s_playing_exit = false;
static bool s_playing_pause = false;

typedef struct {
    AVFormatContext *fmt_ctx;
    AVCodecContext  *dec_ctx;
    int              video_idx;
    int              frame_rate;
}   input_ctx_t;

// 通过interval参数传入当前的timer interval，返回下一次timer的interval，返回0表示取消定时器
// 定时器超时时间到时调用此回调函数，产生FF_REFRESH_EVENT事件，添加到事件队列
static uint32_t sdl_time_cb_refresh(uint32_t interval, void *opaque)
{
    SDL_Event sdl_event;
    sdl_event.type = SDL_USEREVENT_REFRESH;
    SDL_PushEvent(&sdl_event);  // 将事件添加到事件队列，此队列可读可写
    return interval;            // 返回0表示停止定时器 
}

static void get_input_vfmt(const input_ctx_t *ictx, input_vfmt_t *vfmt)
{
    vfmt->width = ictx->dec_ctx->width;
    vfmt->height = ictx->dec_ctx->height;
    vfmt->pix_fmt = ictx->dec_ctx->pix_fmt;
    vfmt->sar = ictx->dec_ctx->sample_aspect_ratio;
    vfmt->time_base = ictx->fmt_ctx->streams[ictx->video_idx]->time_base;
    vfmt->frame_rate = ictx->fmt_ctx->streams[ictx->video_idx]->avg_frame_rate;
    av_log(NULL, AV_LOG_INFO, "get video format: "
           "%dx%d, pix_fmt %d, SAR %d/%d, tb {%d, %d}, rate {%d, %d}\n",
           vfmt->width, vfmt->height, vfmt->pix_fmt,
           vfmt->sar.num, vfmt->sar.den,
           vfmt->time_base.num, vfmt->time_base.den,
           vfmt->frame_rate.num, vfmt->frame_rate.den);

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

// test as:
// ./vf_file test.flv -vf transpose=cclock,pad=iw+80:ih:40 
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s [file] -vf [filtergraph]\n", argv[0]);
        exit(1);
    }

    int ret = 0;

    // 初始化SDL子系统
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS))
    {  
        av_log(NULL, AV_LOG_ERROR, "SDL_Init() failed: %s\n", SDL_GetError()); 
        ret = -1;
        goto exit0;
    }

    input_ctx_t in_ctx = { NULL, NULL, -1, 0};
    ret = open_input(argv[1], &in_ctx);
    if (ret < 0)
    {
        goto exit0;
    }

    input_vfmt_t in_vfmt;
    get_input_vfmt(&in_ctx, &in_vfmt);
    
    filter_ctx_t filter_ctx = { NULL, NULL, NULL };
    ret = init_video_filters(argv[3], &in_vfmt, &filter_ctx);
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

    AVPacket packet;
    AVFrame *frame = NULL;
    AVFrame *filt_frame = NULL;
    frame = av_frame_alloc();
    filt_frame = av_frame_alloc();
    if (!frame || !filt_frame)
    {
        perror("Could not allocate frame");
        goto exit3;
    }

    int temp_num = in_vfmt.frame_rate.num;
    int temp_den = in_vfmt.frame_rate.den;
    int interval = (temp_num > 0) ? (temp_den*1000)/temp_num : 40;
    printf("frame rate %d/%d FPS, refresh interval %d ms\n", temp_num/temp_den, interval);

    // 创建视频解码定时刷新线程，此线程为SDL内部线程，调用指定的回调函数
    SDL_AddTimer(interval, sdl_time_cb_refresh, NULL);
    SDL_Event sdl_event;

    while (1)
    {
        SDL_WaitEvent(&sdl_event);
        if (sdl_event.type != SDL_USEREVENT_REFRESH)
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
    close_input(&in_ctx);
exit0:
    return ret;
}

