#ifndef __FRAME_H__
#define __FRAME_H__

#include "player.h"

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
