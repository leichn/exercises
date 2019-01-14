#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "player.h"

int open_audio_stream(AVFormatContext* p_fmt_ctx, AVCodecContext* p_codec_ctx, const int steam_idx);

#endif
