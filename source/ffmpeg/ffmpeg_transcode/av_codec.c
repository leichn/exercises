#include <stdbool.h>
#include "av_codec.h"

//
// retrun 1:                got a frame success
//        AVERROR(EAGAIN):  need more packet
//        AVERROR_EOF:      end of file, decoder has been flushed
//        <0:               error
int av_decode_frame(AVCodecContext *dec_ctx, AVPacket *packet, bool *new_packet, AVFrame *frame)
{
    int ret = AVERROR(EAGAIN);

    while (1)
    {
        // 2. 从解码器接收frame
        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            // 2.1 一个视频packet含一个视频frame
            //     解码器缓存一定数量的packet后，才有解码后的frame输出
            //     frame输出顺序是按pts的顺序，如IBBPBBP
            //     frame->pkt_pos变量是此frame对应的packet在视频文件中的偏移地址，值同pkt.pos
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret >= 0)
            {
                frame->pts = frame->best_effort_timestamp;
                //frame->pts = frame->pkt_dts;
            }
        }
        else if (dec_ctx->codec_type ==  AVMEDIA_TYPE_AUDIO)
        {
            // 2.2 一个音频packet含一至多个音频frame，每次avcodec_receive_frame()返回一个frame，此函数返回。
            //     下次进来此函数，继续获取一个frame，直到avcodec_receive_frame()返回AVERROR(EAGAIN)，
            //     表示解码器需要填入新的音频packet
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret >= 0)
            {
                // 时基转换，从d->avctx->pkt_timebase时基转换到1/frame->sample_rate时基
                AVRational tb = (AVRational){1, frame->sample_rate};
                if (frame->pts != AV_NOPTS_VALUE)
                {
                    frame->pts = av_rescale_q(frame->pts, dec_ctx->pkt_timebase, tb);
                }
            }
        }

        if (ret >= 0)                   // 成功解码得到一个视频帧或一个音频帧，则返回
        {
            return 1;   
        }
        else if (ret == AVERROR_EOF)    // 解码器已冲洗，解码中所有帧已取出
        {
            avcodec_flush_buffers(dec_ctx);
            av_log(NULL, AV_LOG_INFO, "Decoder has been flushed\n");
            return ret;
        }
        else if (ret == AVERROR(EAGAIN))// 解码器需要喂数据
        {
            if (*new_packet)   // 本函数中已向解码器喂过数据，因此需要从文件读取新数据
            {
                av_log(NULL, AV_LOG_INFO, "Decoder need more packet\n");
                return ret;
            }
        }
        else                            // 错误
        {
            av_log(NULL, AV_LOG_ERROR, "Decoder error %d\n", ret);
            return ret;
        }

        /*
        if (packet == NULL || (packet->data == NULL && packet->size == 0))
        {
            // 复位解码器内部状态/刷新内部缓冲区。当seek操作或切换流时应调用此函数。
            avcodec_flush_buffers(dec_ctx);
        }
        */

        // 1. 将packet发送给解码器
        //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
        //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
        //    发送第一个 flush packet 会返回成功，后续的 flush packet 会返回AVERROR_EOF
        ret = avcodec_send_packet(dec_ctx, packet);
        *new_packet = false;
        
        if (ret != 0)
        {
            av_log(NULL, AV_LOG_ERROR, "avcodec_send_packet() error, return %d\n", ret);
            return ret;
        }
    }

    return -1;
}

int av_encode_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *packet)
{
    int ret = -1;
    
    // 第一次发送flush packet会返回成功，进入刷洗模式，可调用avcodec_receive_packet()
    // 将编码器中缓存的帧(可能不止一个)取出来
    // 后续再发送flush packet将返回AVERROR_EOF
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret == AVERROR_EOF)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() encoder flushed\n");
    }
    else if (ret == AVERROR(EAGAIN))
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() need output read out\n");
    }
    else if (ret < 0)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() error %d\n", ret);
        return ret;
    }

    ret = avcodec_receive_packet(enc_ctx, packet);
    if (ret == AVERROR_EOF)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_receive_packet() encoder flushed\n");
    }
    else if (ret == AVERROR(EAGAIN))
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_receive_packet() need more input\n");
    }
    
    return ret;
}

