/*******************************************************************************
 * player.c
 *
 * history:
 *   2018-11-27 - [lei]     Create file: a simplest ffmpeg player
 *   2018-12-01 - [lei]     Playing audio
 *   2018-12-06 - [lei]     Playing audio&vidio
 *   2019-01-06 - [lei]     Add audio resampling, fix bug of unsupported audio 
 *                          format(such as planar)
 *
 * details:
 *   A simple ffmpeg player.
 *
 * refrence:
 *   1. https://blog.csdn.net/leixiaohua1020/article/details/38868499
 *   2. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial01.html
 *   3. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial02.html
 *   4. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial03.html
 *   5. http://dranger.com/ffmpeg/ffmpegtutorial_all.html#tutorial04.html
 *******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>

#include "frame.h"
#include "packet.h"
#include "ffplayer.h"

#define SDL_USEREVENT_REFRESH  (SDL_USEREVENT + 1)

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

static packet_queue_t s_audio_pkt_queue;
static FF_AudioParams s_audio_param_src;
static FF_AudioParams s_audio_param_tgt;
static struct SwrContext *s_audio_swr_ctx;
static uint8_t *s_resample_buf = NULL;  // 重采样输出缓冲区
static int s_resample_buf_len = 0;      // 重采样输出缓冲区长度

static bool s_input_finished = false;   // 文件读取完毕
static bool s_adecode_finished = false; // 解码完毕
static bool s_vdecode_finished = false; // 解码完毕

static packet_queue_t s_audio_pkt_queue;
static packet_queue_t s_video_pkt_queue;

// 返回值：如果按正常速度播放，则返回上一帧的pts；若是快进或快退播放，则返回上一帧的pts经校正后的值
static double get_clock(clock_t *c)
{
    if (*c->queue_serial != c->serial)
        return NAN;
    if (c->paused) {
        return c->pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        // 设c->speed为0，则展开得：(c->pts - c->last_updated) + time - (time - c->last_updated)*1 ＝ c->pts
        return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
    }
}

static void set_clock_at(clock_t *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

static void set_clock(clock_t *c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

static void set_clock_speed(clock_t *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

static void init_clock(clock_t *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

static void sync_clock_to_slave(clock_t *c, clock_t *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, slave->serial);
}

static int get_master_sync_type(player_stat_t *is) {
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

// 通过interval参数传入当前的timer interval，返回下一次timer的interval，返回0表示取消定时器
// 定时器超时时间到时调用此回调函数，产生FF_REFRESH_EVENT事件，添加到事件队列
static uint32_t sdl_time_cb_refresh(uint32_t interval, void *opaque)
{
    SDL_Event sdl_event;
    sdl_event.type = SDL_USEREVENT_REFRESH;
    SDL_PushEvent(&sdl_event);  // 将事件添加到事件队列，此队列可读可写
    return interval;            // 返回0表示停止定时器 
}


static void do_exit(player_stat_t *is)
{
    if (is)
    {
        player_deinit(is);
    }

    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    
    avformat_network_deinit();

    SDL_Quit();

    exit(0);
}

static player_stat_t *player_init(void)
{
    player_stat_t *is;

    is = av_mallocz(sizeof(player_stat_t));
    if (!is)
    {
        return NULL;
    }

    is->ytop    = 0;
    is->xleft   = 0;

    /* start video display */
    if (frame_queue_init(&is->pictq, &is->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0 ||
        frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
    {
        goto fail;
    }

    if (packet_queue_init(&is->videoq) < 0 ||
        packet_queue_init(&is->audioq) < 0)
    {
        goto fail;
    }

    if (!(is->continue_read_thread = SDL_CreateCond()))
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        goto fail;
    }

    init_clock(&is->vidclk, &is->videoq.serial);
    init_clock(&is->audclk, &is->audioq.serial);
    init_clock(&is->extclk, &is->extclk.serial);

    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is);
    if (!is->read_tid)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread(): %s\n", SDL_GetError());
fail:
        player_deinit(is);
        return NULL;
    }

    return is;
}


int player_deinit(player_stat_t *is)
{

}


int player_running(const char *p_input_file)
{
    player_stat_t *is = NULL;

    is = player_init();
    if (is == NULL)
    {
        printf("player init failed\n");
        do_exit(is);
    }

    demux_running();
    video_decode_running();
    video_playing_runing();
    audio_decode_runing();
    audio_playing_runing();

    while (1)
    {

    }

    return 0;
}
