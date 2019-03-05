#ifndef __AV_CODEC_H__
#define __AV_CODEC_H__

#include <stdbool.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>

int av_decode_frame(AVCodecContext *dec_ctx, AVPacket *packet, bool *new_packet, AVFrame *frame);
int av_encode_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *packet);

#endif

