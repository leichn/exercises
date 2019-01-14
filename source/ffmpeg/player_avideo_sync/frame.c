#include "frame.h"
#include "player.h"

void frame_queue_unref_item(frame_t *vp)
{
    av_frame_unref(vp->frame);
}

int frame_queue_init(frame_queue_t *f, packet_queue_t *pktq, int max_size, int keep_last)
{
    int i;
    memset(f, 0, sizeof(frame_queue_t));
    if (!(f->mutex = SDL_CreateMutex())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    if (!(f->cond = SDL_CreateCond())) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = !!keep_last;
    for (i = 0; i < f->max_size; i++)
        if (!(f->queue[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    return 0;
}

void frame_queue_destory(frame_queue_t *f)
{
    int i;
    for (i = 0; i < f->max_size; i++) {
        frame_t *vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

void frame_queue_signal(frame_queue_t *f)
{
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

frame_t *frame_queue_peek(frame_queue_t *f)
{
    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

frame_t *frame_queue_peek_next(frame_queue_t *f)
{
    return &f->queue[(f->rindex + f->rindex_shown + 1) % f->max_size];
}

// 取出此帧进行播放，只读取不删除，不删除是因为此帧需要缓存下来供下一次使用。播放后，此帧变为上一帧
frame_t *frame_queue_peek_last(frame_queue_t *f)
{
    return &f->queue[f->rindex];
}

// 向队列尾部申请一个可写的帧空间，若无空间可写，则等待
frame_t *frame_queue_peek_writable(frame_queue_t *f)
{
    /* wait until we have space to put a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[f->windex];
}

// 从队列头部读取一帧，只读取不删除，若无帧可读则等待
frame_t *frame_queue_peek_readable(frame_queue_t *f)
{
    /* wait until we have a readable a new frame */
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

// 向队列尾部压入一帧，只更新计数与写指针，因此调用此函数前应将帧数据写入队列相应位置
void frame_queue_push(frame_queue_t *f)
{
    if (++f->windex == f->max_size)
        f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

// 读指针(rindex)指向的帧已显示，删除此帧，注意不读取直接删除。读指针加1
void frame_queue_next(frame_queue_t *f)
{
    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size)
        f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

// frame_queue中未显示的帧数
/* return the number of undisplayed frames in the queue */
int frame_queue_nb_remaining(frame_queue_t *f)
{
    return f->size - f->rindex_shown;
}

/* return last shown position */
int64_t frame_queue_last_pos(frame_queue_t *f)
{
    frame_t *fp = &f->queue[f->rindex];
    if (f->rindex_shown && fp->serial == f->pktq->serial)
        return fp->pos;
    else
        return -1;
}

