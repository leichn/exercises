#ifndef __FRAME_H__
#define __FRAME_H__

#include <libavutil/frame.h>
#include <SDL2/SDL_mutex.h>

#include "packet.h"

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct {
    AVFrame *frame;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;                    // frame对应的packet在输入文件中的地址偏移
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
}   frame_t;

typedef struct {
    frame_t queue[FRAME_QUEUE_SIZE];
    int rindex;                     // 读索引。待播放时读取此帧进行播放，播放后此帧成为上一帧
    int windex;                     // 写索引。
    int size;                       // 总帧数
    int max_size;                   // 队列可存储最大帧数
    int keep_last;
    int rindex_shown;               // 当前是否有帧在显示
    SDL_mutex *mutex;
    SDL_cond *cond;
    packet_queue_t *pktq;           // 指向对应的packet_queue
}   frame_queue_t;

void frame_queue_unref_item(frame_t *vp);
int frame_queue_init(frame_queue_t *f, packet_queue_t *pktq, int max_size, int keep_last);
void frame_queue_destory(frame_queue_t *f);
void frame_queue_signal(frame_queue_t *f);
frame_t *frame_queue_peek(frame_queue_t *f);
frame_t *frame_queue_peek_next(frame_queue_t *f);
frame_t *frame_queue_peek_last(frame_queue_t *f);
frame_t *frame_queue_peek_writable(frame_queue_t *f);
frame_t *frame_queue_peek_readable(frame_queue_t *f);
void frame_queue_push(frame_queue_t *f);
void frame_queue_next(frame_queue_t *f);
int frame_queue_nb_remaining(frame_queue_t *f);
int64_t frame_queue_last_pos(frame_queue_t *f);

#endif
