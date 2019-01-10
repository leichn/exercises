#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

#include "packet.h"
#include "frame.h"

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30


#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

typedef struct {
    double pts;                     // 当前帧(待播放)显示时间戳，播放后，当前帧变成上一帧
    double pts_drift;               // 当前帧显示时间戳与当前系统时钟时间的差值
    double last_updated;            // 当前时钟(如视频时钟)最后一次更新时间，也可称当前时钟时间
    double speed;                   // 时钟速度控制，用于控制播放速度
    int serial;                     // 播放序列，所谓播放序列就是一段连续的播放动作，一个seek操作会启动一段新的播放序列
    int paused;                     // 暂停标志
    int *queue_serial;              // 指向packet_serial
}   play_clock_t;

typedef struct {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
}   audio_param_t;

typedef struct {
    SDL_Window *window; 
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Rect rect;
}   sdl_video_t;

typedef struct {
    char *filename;
    AVFormatContext *p_fmt_ctx;
    AVStream *p_audio_stream;
    AVStream *p_video_stream;
    AVCodecContext *p_audio_codec_ctx;
    AVCodecContext *p_video_codec_ctx;

    int audio_idx;
    int video_idx;
    sdl_video_t sdl_video;

    play_clock_t audio_clk;                   // 音频时钟
    play_clock_t video_clk;                   // 视频时钟
    double frame_timer;

    packet_queue_t audio_pkt_queue;
    packet_queue_t video_pkt_queue;
    frame_queue_t audio_frm_queue;
    frame_queue_t video_frm_queue;

    audio_param_t audio_param_src;
    audio_param_t audio_param_tgt;
    struct SwrContext *swr_ctx;
    int audio_hw_buf_size;          // SDL音频缓冲区大小(单位字节)
    uint8_t *audio_buf;             // 指向待播放的一帧音频数据，指向的数据区将被拷入SDL音频缓冲区。若经过重采样则指向audio_buf1，否则指向frame中的音频
    uint8_t *audio_buf1;            // 音频重采样的输出缓冲区
    unsigned int audio_buf_size; /* in bytes */ // 待播放的一帧音频数据(audio_buf指向)的大小
    unsigned int audio_buf1_size;   // 申请到的音频缓冲区audio_buf1的实际尺寸
    int audio_buf_index; /* in bytes */ // 当前音频帧中已拷入SDL音频缓冲区的位置索引(指向第一个待拷贝字节)
    int audio_write_buf_size;       // 当前音频帧中尚未拷入SDL音频缓冲区的数据量，audio_buf_size = audio_buf_index + audio_write_buf_size
    double audio_clock;
    int audio_clock_serial;
    
    int abort_request;

    SDL_cond *continue_read_thread;

}   player_stat_t;


int player_running(const char *p_input_file);
double get_clock(play_clock_t *c);
void set_clock_at(play_clock_t *c, double pts, int serial, double time);
void set_clock(play_clock_t *c, double pts, int serial);

#endif
