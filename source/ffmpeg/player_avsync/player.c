/*******************************************************************************
 * player.c
 *
 * history:
 *   2018-11-27 - [lei]     Create file: a simplest ffmpeg player
 *   2018-12-01 - [lei]     Playing audio
 *   2018-12-06 - [lei]     Playing audio&vidio
 *   2019-01-06 - [lei]     Add audio resampling, fix bug of unsupported audio 
 *                          format(such as planar)
 *   2019-01-16 - [lei]     Sync video to audio.
 *
 * details:
 *   A simple ffmpeg player.
 *
 * refrence:
 *   ffplay.c in FFmpeg 4.1 project.
 *******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "player.h"
#include "frame.h"
#include "packet.h"
#include "demux.h"
#include "video.h"
#include "audio.h"

static player_stat_t *player_init(const char *p_input_file);
static int player_deinit(player_stat_t *is);

// 返回值：返回上一帧的pts更新值(上一帧pts+流逝的时间)
double get_clock(play_clock_t *c)
{
    if (*c->queue_serial != c->serial)
    {
        return NAN;
    }
    if (c->paused)
    {
        return c->pts;
    }
    else
    {
        double time = av_gettime_relative() / 1000000.0;
        double ret = c->pts_drift + time;   // 展开得： c->pts + (time - c->last_updated)
        return ret;
    }
}

void set_clock_at(play_clock_t *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

void set_clock(play_clock_t *c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}

static void set_clock_speed(play_clock_t *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

void init_clock(play_clock_t *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

static void sync_play_clock_to_slave(play_clock_t *c, play_clock_t *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, slave->serial);
}

static void do_exit(player_stat_t *is)
{
    if (is)
    {
        player_deinit(is);
    }

    if (is->sdl_video.renderer)
        SDL_DestroyRenderer(is->sdl_video.renderer);
    if (is->sdl_video.window)
        SDL_DestroyWindow(is->sdl_video.window);
    
    avformat_network_deinit();

    SDL_Quit();

    exit(0);
}

static player_stat_t *player_init(const char *p_input_file)
{
    player_stat_t *is;

    is = av_mallocz(sizeof(player_stat_t));
    if (!is)
    {
        return NULL;
    }

    is->filename = av_strdup(p_input_file);
    if (is->filename == NULL)
    {
        goto fail;
    }

    /* start video display */
    if (frame_queue_init(&is->video_frm_queue, &is->video_pkt_queue, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0 ||
        frame_queue_init(&is->audio_frm_queue, &is->audio_pkt_queue, SAMPLE_QUEUE_SIZE, 1) < 0)
    {
        goto fail;
    }

    if (packet_queue_init(&is->video_pkt_queue) < 0 ||
        packet_queue_init(&is->audio_pkt_queue) < 0)
    {
        goto fail;
    }

    AVPacket flush_pkt;
    flush_pkt.data = NULL;
    packet_queue_put(&is->video_pkt_queue, &flush_pkt);
    packet_queue_put(&is->audio_pkt_queue, &flush_pkt);

    if (!(is->continue_read_thread = SDL_CreateCond()))
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
fail:
        player_deinit(is);
        goto fail;
    }

    init_clock(&is->video_clk, &is->video_pkt_queue.serial);
    init_clock(&is->audio_clk, &is->audio_pkt_queue.serial);

    is->abort_request = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        av_log(NULL, AV_LOG_FATAL, "Could not initialize SDL - %s\n", SDL_GetError());
        av_log(NULL, AV_LOG_FATAL, "(Did you set the DISPLAY variable?)\n");
        exit(1);
    }

    return is;
}


static int player_deinit(player_stat_t *is)
{
    /* XXX: use a special url_shutdown call to abort parse cleanly */
    is->abort_request = 1;
    SDL_WaitThread(is->read_tid, NULL);

    /* close each stream */
    if (is->audio_idx >= 0)
    {
        //stream_component_close(is, is->p_audio_stream);
    }
    if (is->video_idx >= 0)
    {
        //stream_component_close(is, is->p_video_stream);
    }

    avformat_close_input(&is->p_fmt_ctx);

    packet_queue_abort(&is->video_pkt_queue);
    packet_queue_abort(&is->audio_pkt_queue);
    packet_queue_destroy(&is->video_pkt_queue);
    packet_queue_destroy(&is->audio_pkt_queue);

    /* free all pictures */
    frame_queue_destory(&is->video_frm_queue);
    frame_queue_destory(&is->audio_frm_queue);

    SDL_DestroyCond(is->continue_read_thread);
    sws_freeContext(is->img_convert_ctx);
    av_free(is->filename);
    if (is->sdl_video.texture)
    {
        SDL_DestroyTexture(is->sdl_video.texture);
    }

    av_free(is);

    return 0;
}

/* pause or resume the video */
static void stream_toggle_pause(player_stat_t *is)
{
    if (is->paused)
    {
        // 这里表示当前是暂停状态，将切换到继续播放状态。在继续播放之前，先将暂停期间流逝的时间加到frame_timer中
        is->frame_timer += av_gettime_relative() / 1000000.0 - is->video_clk.last_updated;
        set_clock(&is->video_clk, get_clock(&is->video_clk), is->video_clk.serial);
    }
    is->paused = is->audio_clk.paused = is->video_clk.paused = !is->paused;
}

static void toggle_pause(player_stat_t *is)
{
    stream_toggle_pause(is);
    is->step = 0;
}

int player_running(const char *p_input_file)
{
    player_stat_t *is = NULL;

    is = player_init(p_input_file);
    if (is == NULL)
    {
        printf("player init failed\n");
        do_exit(is);
    }

    open_demux(is);
    open_video(is);
    open_audio(is);

    SDL_Event event;

    while (1)
    {
        SDL_PumpEvents();
        // SDL event队列为空，则在while循环中播放视频帧。否则从队列头部取一个event，退出当前函数，在上级函数中处理event
        while (!SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
        {
            av_usleep(100000);
            SDL_PumpEvents();
        }

        switch (event.type) {
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                do_exit(is);
                break;
            }

            switch (event.key.keysym.sym) {
            case SDLK_SPACE:        // 空格键：暂停
                toggle_pause(is);
                break;
            case SDL_WINDOWEVENT:
                break;
            default:
                break;
            }
            break;

        case SDL_QUIT:
        case FF_QUIT_EVENT:
            do_exit(is);
            break;
        default:
            break;
        }
    }

    return 0;
}
