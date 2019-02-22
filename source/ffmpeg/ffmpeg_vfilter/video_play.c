#include <libswscale/swscale.h>
#include "video_play.h"

static int open_sdl_window(disp_ctx_t *dctx, int w, int h)
{
    SDL_SetWindowTitle(dctx->sdl_window, "simple ffplayer with video filter");
    SDL_SetWindowSize(dctx->sdl_window, w, h);
    SDL_SetWindowPosition(dctx->sdl_window, 0, 0);
    SDL_ShowWindow(dctx->sdl_window);

    dctx->win_opened = true;

    return 0;
}

// 若有必要(显示参数有变或texture无效)则重建texture
static int realloc_texture(SDL_Renderer *renderer, SDL_Texture **texture, 
                           uint32_t new_format, int new_width, int new_height,
                           SDL_BlendMode blendmode, int init_texture)
{
    uint32_t format;
    int access, w, h;
    // 1) texture未曾建立 2) 查询texture属性无效 3) 宽、高、像素格式有变化，这三种情况下需要重建texture
    if (!*texture || 
        SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || 
        new_width != w || new_height != h || new_format != format)
    {
        void *pixels;
        int pitch;
        if (*texture)
        {
            SDL_DestroyTexture(*texture);
        }
        if (!(*texture = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)))
        {
            return -1;
        }
        if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
        {
            return -1;
        }
        if (init_texture)
        {
            if (SDL_LockTexture(*texture, NULL, &pixels, &pitch) < 0)
            {
                return -1;
            }
            memset(pixels, 0, pitch * new_height);
            SDL_UnlockTexture(*texture);
        }
        av_log(NULL, AV_LOG_VERBOSE, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
    }
    return 0;
}

// 设置YUV与RGB转换模式，此模式在SDL_RenderCopyEx中会用到
static void set_sdl_yuv_conversion_mode(AVFrame *frame)
{
#if SDL_VERSION_ATLEAST(2,0,8)
    SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
    if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
        if (frame->color_range == AVCOL_RANGE_JPEG)     // color_range区分是JPEG(图片)还是MPEG(视频)
            mode = SDL_YUV_CONVERSION_JPEG;
        else if (frame->colorspace == AVCOL_SPC_BT709)  // colorspace是由ISO标准定义的YUV色彩空间类型
            mode = SDL_YUV_CONVERSION_BT709;            // HDTV
        else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M || frame->colorspace == AVCOL_SPC_SMPTE240M)
            mode = SDL_YUV_CONVERSION_BT601;            // SDTV
    }
    SDL_SetYUVConversionMode(mode);
#endif
}

static const av_sdl_fmt_map_t sdl_texture_format_map[] = {
    { AV_PIX_FMT_RGB8,           SDL_PIXELFORMAT_RGB332 },
    { AV_PIX_FMT_RGB444,         SDL_PIXELFORMAT_RGB444 },
    { AV_PIX_FMT_RGB555,         SDL_PIXELFORMAT_RGB555 },
    { AV_PIX_FMT_BGR555,         SDL_PIXELFORMAT_BGR555 },
    { AV_PIX_FMT_RGB565,         SDL_PIXELFORMAT_RGB565 },
    { AV_PIX_FMT_BGR565,         SDL_PIXELFORMAT_BGR565 },
    { AV_PIX_FMT_RGB24,          SDL_PIXELFORMAT_RGB24 },
    { AV_PIX_FMT_BGR24,          SDL_PIXELFORMAT_BGR24 },
    { AV_PIX_FMT_0RGB32,         SDL_PIXELFORMAT_RGB888 },
    { AV_PIX_FMT_0BGR32,         SDL_PIXELFORMAT_BGR888 },
    { AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888 },
    { AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888 },
    { AV_PIX_FMT_RGB32,          SDL_PIXELFORMAT_ARGB8888 },
    { AV_PIX_FMT_RGB32_1,        SDL_PIXELFORMAT_RGBA8888 },
    { AV_PIX_FMT_BGR32,          SDL_PIXELFORMAT_ABGR8888 },
    { AV_PIX_FMT_BGR32_1,        SDL_PIXELFORMAT_BGRA8888 },
    { AV_PIX_FMT_YUV420P,        SDL_PIXELFORMAT_IYUV },
    { AV_PIX_FMT_YUYV422,        SDL_PIXELFORMAT_YUY2 },
    { AV_PIX_FMT_UYVY422,        SDL_PIXELFORMAT_UYVY },
    { AV_PIX_FMT_NONE,           SDL_PIXELFORMAT_UNKNOWN },
};



// 获取输入参数format(FFmpeg像素格式)在SDL中的像素格式，取到的SDL像素格式存在输出参数sdl_pix_fmt中
static void get_sdl_pix_fmt_and_blendmode(int format, uint32_t *sdl_pix_fmt, SDL_BlendMode *sdl_blendmode)
{
    int i;
    *sdl_blendmode = SDL_BLENDMODE_NONE;
    *sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
    if (format == AV_PIX_FMT_RGB32   ||
        format == AV_PIX_FMT_RGB32_1 ||
        format == AV_PIX_FMT_BGR32   ||
        format == AV_PIX_FMT_BGR32_1)           // 这些格式含A(alpha)通道
    {
        *sdl_blendmode = SDL_BLENDMODE_BLEND;   // alpha混合模式
    }
        
    for (i = 0; i < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; i++)
    {
        if (format == sdl_texture_format_map[i].format)
        {
            *sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
            return;
        }
    }
}

static int upload_texture(SDL_Renderer *renderer, SDL_Texture **tex, AVFrame *frame, struct SwsContext **sws_ctx)
{
    int ret = 0;
    uint32_t sdl_pix_fmt;
    SDL_BlendMode sdl_blendmode;
    
    // 根据frame中的图像格式(FFmpeg像素格式)，获取对应的SDL像素格式
    get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
    
    // 参数tex实际是&is->vid_texture，此处根据得到的SDL像素格式，为&is->vid_texture重新分配内存空间
    uint32_t new_fmt = (sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN) ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt;
    if (realloc_texture(renderer, tex, new_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
    {
        return -1;
    }
    
    switch (sdl_pix_fmt)
    {
        // frame格式是SDL不支持的格式，则需要进行图像格式转换，转换为目标格式AV_PIX_FMT_BGRA(FFmpeg)，对应SDL_PIXELFORMAT_BGRA32(SDL)
        case SDL_PIXELFORMAT_UNKNOWN:
            /* This should only happen if we are not using avfilter... */
            // 若能复用现有SwsContext则复用，否则重新分配一个
            *sws_ctx = 
            sws_getCachedContext(*sws_ctx,
                                 frame->width, frame->height, frame->format, frame->width, frame->height,
                                 AV_PIX_FMT_BGRA, SWS_BICUBIC, NULL, NULL, NULL);
            if (*sws_ctx != NULL)
            {
                uint8_t *pixels[4];
                int pitch[4];
                if (!SDL_LockTexture(*tex, NULL, (void **)pixels, pitch))
                {
                    sws_scale(*sws_ctx, (const uint8_t * const *)frame->data, frame->linesize,
                              0, frame->height, pixels, pitch);
                    SDL_UnlockTexture(*tex);
                }
            }
            else
            {
                av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
                ret = -1;
            }
            break;
            
        // frame格式对应SDL_PIXELFORMAT_IYUV，不用进行图像格式转换，调用SDL_UpdateYUVTexture()更新SDL texture
        case SDL_PIXELFORMAT_IYUV:
            if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0)
            {
                int ypitch = frame->linesize[0];
                int upitch = frame->linesize[1];
                int vpitch = frame->linesize[2];
                uint8_t *yplane = frame->data[0];
                uint8_t *uplane = frame->data[1];
                uint8_t *vplane = frame->data[2];                
                ret = SDL_UpdateYUVTexture(*tex, NULL, yplane, ypitch, uplane, upitch, vplane, vpitch);
            }
            else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0)
            {
                int ypitch = -frame->linesize[0];
                int upitch = -frame->linesize[1];
                int vpitch = -frame->linesize[2];
                uint8_t *yplane = frame->data[0] + frame->linesize[0] * (frame->height - 1);
                uint8_t *uplane = frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1);
                uint8_t *vplane = frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1);
                ret = SDL_UpdateYUVTexture(*tex, NULL, yplane, ypitch, uplane, upitch, vplane, vpitch);
            }
            else
            {
                av_log(NULL, AV_LOG_ERROR, "Mixed negative and positive linesizes are not supported.\n");
                return -1;
            }
            break;
            
        // frame格式对应其他SDL支持的像素格式，不用进行图像格式转换，调用SDL_UpdateTexture()更新SDL texture
        default:
            if (frame->linesize[0] < 0)
            {
                ret = SDL_UpdateTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
            }
            else
            {
                ret = SDL_UpdateTexture(*tex, NULL, frame->data[0], frame->linesize[0]);
            }
            break;
    }
    return ret;
}


static void video_image_display(disp_ctx_t *dctx, AVFrame *frame)
{
    // 1. 计算显示区域SDL_Rect的大小
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = frame->width;
    rect.h = frame->height;

    // 2. 使用一帧视频帧数据更新SDL_Texture
    if (upload_texture(dctx->sdl_renderer, &dctx->sdl_texture, frame, &dctx->sws_ctx) < 0)
    {
        return;
    }

    // 设置YUV与RGB转换模式，此模式在SDL_RenderCopyEx中会用到
    set_sdl_yuv_conversion_mode(frame);
    // 将texture中的部分图像(rect部分)拷贝到renderer
    SDL_RenderCopyEx(dctx->sdl_renderer, dctx->sdl_texture, NULL, &rect, 0, NULL, 0);
    set_sdl_yuv_conversion_mode(NULL);
}

int init_displaying(disp_ctx_t *dctx)
{
    int ret;

    // 创建SDL窗口，SDL 2.0支持多窗口
    // SDL_Window即运行程序后弹出的视频窗口，同SDL 1.x中的SDL_Surface
    int default_width = 640;
    int default_height = 480;
    dctx->sdl_window = 
    SDL_CreateWindow("simple ffplayer with video filter", 
                     SDL_WINDOWPOS_UNDEFINED,   // 不关心窗口X坐标
                     SDL_WINDOWPOS_UNDEFINED,   // 不关心窗口Y坐标
                     default_width,
                     default_height,
                     SDL_WINDOW_HIDDEN          // 窗口先不显示
                    );

    if (dctx->sdl_window == NULL)
    {  
        av_log(NULL, AV_LOG_ERROR, "SDL_CreateWindow() failed: %s\n", SDL_GetError());  
        ret = -1;
        goto exit1;
    }

    // 创建SDL_Renderer
    // SDL_Renderer：渲染器
    dctx->sdl_renderer = 
    SDL_CreateRenderer(dctx->sdl_window, -1, 
                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (dctx->sdl_renderer == NULL)
    {
        av_log(NULL, AV_LOG_WARNING, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
        dctx->sdl_renderer = SDL_CreateRenderer(dctx->sdl_window, -1, 0);     
        if (dctx->sdl_renderer == NULL)
        {  
            av_log(NULL, AV_LOG_ERROR, "SDL_CreateRenderer() failed: %s\n", SDL_GetError());  
            ret = -1;
            goto exit1;
        }
    }

    return 0;

exit1:
    SDL_Quit();
exit0:
    return ret;
}

int deinit_displaying(disp_ctx_t *dctx)
{
    sws_freeContext(dctx->sws_ctx);
    SDL_Quit();

    return 0;
}

/* display the current picture, if any */
void play_video_frame(disp_ctx_t *dctx, AVFrame *frame)
{
    if (!dctx->win_opened)
    {
        open_sdl_window(dctx, frame->width, frame->height);
    }

    SDL_SetRenderDrawColor(dctx->sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(dctx->sdl_renderer);
    video_image_display(dctx, frame);
    SDL_RenderPresent(dctx->sdl_renderer);
}

