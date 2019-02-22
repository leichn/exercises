#ifndef __VIDEO_PLAY_H__
#define __VIDEO_PLAY_H__

#include <stdbool.h>
#include <libavutil/pixfmt.h>
#include <libavutil/frame.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>

typedef struct {
    enum AVPixelFormat format;
    int texture_fmt;
}   av_sdl_fmt_map_t;

typedef struct {
    SDL_Window      *sdl_window; 
    SDL_Renderer    *sdl_renderer;
    SDL_Texture     *sdl_texture;
    struct SwsContext *sws_ctx;
    bool            win_opened;
}   disp_ctx_t;

int init_displaying(disp_ctx_t *dctx);
int deinit_displaying(disp_ctx_t *dctx);
void play_video_frame(disp_ctx_t *dctx, AVFrame *frame);

#endif

