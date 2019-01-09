#ifndef __DEMUX_H__
#define __DEMUX_H__

#include "player.h"

typedef struct
{
    AVFormatContext *p_ic;
    int aud_idx;
    int vid_idx;
    AVStream *p_aud_st;
    AVStream *p_vid_st;
    packet_queue_t *p_aud_pkt_q;
    packet_queue_t *p_vid_pkt_q;
}   player_demux_t;

#endif
