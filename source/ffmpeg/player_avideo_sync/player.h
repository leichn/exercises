#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#include "demux.h"

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

typedef struct {
    double pts;                     // 当前帧(待播放)显示时间戳，播放后，当前帧变成上一帧
    double pts_drift;               // 当前帧显示时间戳与当前系统时钟时间的差值
    double last_updated;            // 当前时钟(如视频时钟)最后一次更新时间，也可称当前时钟时间
    double speed;                   // 时钟速度控制，用于控制播放速度
    int serial;                     // 播放序列，所谓播放序列就是一段连续的播放动作，一个seek操作会启动一段新的播放序列
    int paused;                     // 暂停标志
    int *queue_serial;              // 指向packet_serial
}   clock_t;

#if 0
typedef struct {
    SDL_Thread *read_tid;           // demux解复用线程
    AVInputFormat *iformat;
    int abort_request;
    int force_refresh;
    int paused;
    int last_paused;
    int queue_attachments_req;

    AVFormatContext *ic;
    int realtime;

    clock_t audclk;                   // 音频时钟
    clock_t vidclk;                   // 视频时钟
    clock_t extclk;                   // 外部时钟

    frame_queue_t picq;             // 视频frame队列
    frame_queue_t audq;             // 音频frame队列

    Decoder auddec;                 // 音频解码器
    Decoder viddec;                 // 视频解码器

    int audio_stream;               // 音频流索引

    double audio_clock;             // 每个音频帧更新一下此值，以pts形式表示
    int audio_clock_serial;         // 播放序列，seek可改变此值
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;             // 音频流
    packet_queue_t audioq;             // 音频packet队列
    int audio_hw_buf_size;          // SDL音频缓冲区大小(单位字节)
    uint8_t *audio_buf;             // 指向待播放的一帧音频数据，指向的数据区将被拷入SDL音频缓冲区。若经过重采样则指向audio_buf1，否则指向frame中的音频
    uint8_t *audio_buf1;            // 音频重采样的输出缓冲区
    unsigned int audio_buf_size; /* in bytes */ // 待播放的一帧音频数据(audio_buf指向)的大小
    unsigned int audio_buf1_size;   // 申请到的音频缓冲区audio_buf1的实际尺寸
    int audio_buf_index; /* in bytes */ // 当前音频帧中已拷入SDL音频缓冲区的位置索引(指向第一个待拷贝字节)
    int audio_write_buf_size;       // 当前音频帧中尚未拷入SDL音频缓冲区的数据量，audio_buf_size = audio_buf_index + audio_write_buf_size
    int audio_volume;               // 音量
    int muted;                      // 静音状态
    struct AudioParams audio_src;   // 音频frame的参数
    struct AudioParams audio_tgt;   // SDL支持的音频参数，重采样转换：audio_src->audio_tgt
    struct SwrContext *swr_ctx;     // 音频重采样context
    int frame_drops_early;          // 丢弃视频packet计数
    int frame_drops_late;           // 丢弃视频frame计数

    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;
    int xpos;
    double last_vis_time;
    SDL_Texture *vis_texture;
    SDL_Texture *sub_texture;
    SDL_Texture *vid_texture;

    double frame_timer;             // 记录最后一帧播放的时刻
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int video_stream;
    AVStream *video_st;             // 视频流
    packet_queue_t videoq;          // 视频队列
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    struct SwsContext *img_convert_ctx;
    int eof;

    char *filename;
    int width, height, xleft, ytop;
    int step;

    SDL_cond *continue_read_thread;
}player_stat_t;
#else
typedef struct {

    clock_t audclk;                   // 音频时钟
    clock_t vidclk;                   // 视频时钟
    clock_t extclk;                   // 外部时钟

    packet_queue_t aud_pkt_q;
    packet_queue_t vid_pkt_q;
    frame_queue_t aud_frm_q;
    frame_queue_t vid_frm_q;
}player_stat_t;
#endif

int player_running(const char *p_input_file);

#endif
