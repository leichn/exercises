#ifndef __OPEN_FILE_H__
#define __OPEN_FILE_H__

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>

typedef struct {
    AVFormatContext* fmt_ctx;
    AVCodecContext** codec_ctx;     // AVCodecContext* codec_ctx[];
}   inout_ctx_t;

int open_input_file(const char *filename, inout_ctx_t *ictx);
int open_output_file(const char *filename, const inout_ctx_t *ictx, 
                     const char *v_enc_name, const char *a_enc_name,
                     inout_ctx_t *octx, AVAudioFifo*** afifo);

#endif

