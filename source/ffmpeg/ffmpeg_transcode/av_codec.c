
//
// retrun 1: got a frame success
//        0: need mored packet
//        AVERROR_EOF: 
//        <0: error
int av_decode_frame(AVCodecContext *dec_ctx, AVPacket *packet, AVFrame *frame)
{
    int ret = AVERROR(EAGAIN);

    while (1)
    {
        // 3. 从解码器接收frame
        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            // 3.1 一个视频packet含一个视频frame
            //     解码器缓存一定数量的packet后，才有解码后的frame输出
            //     frame输出顺序是按pts的顺序，如IBBPBBP
            //     frame->pkt_pos变量是此frame对应的packet在视频文件中的偏移地址，值同pkt.pos
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret >= 0)
            {
                frame->pts = frame->best_effort_timestamp;
                frame->pts = frame->pkt_dts;
            }
        }
        else if (dec_ctx->codec_type ==  AVMEDIA_TYPE_AUDIO)
        {
            // 3.2 一个音频packet含一至多个音频frame，每次avcodec_receive_frame()返回一个frame，此函数返回。
            // 下次进来此函数，继续获取一个frame，直到avcodec_receive_frame()返回AVERROR(EAGAIN)，
            // 表示解码器需要填入新的音频packet
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

        if (ret >= 0)
        {
            return 1;   // 成功解码得到一个视频帧或一个音频帧，则返回
        }
        else if (ret == AVERROR_EOF)
        {
            avcodec_flush_buffers(dec_ctx);
            av_log(NULL, AV_LOG_INFO, "Decoder has been flushed\n");
            return ret;
        }
        else if (ret == AVERROR(EAGAIN))
        {
        }


        /*
        if (packet == NULL)
        {
            // 复位解码器内部状态/刷新内部缓冲区。当seek操作或切换流时应调用此函数。
            avcodec_flush_buffers(dec_ctx);
        }
        */

        // 2. 将packet发送给解码器
        //    发送packet的顺序是按dts递增的顺序，如IPBBPBB
        //    pkt.pos变量可以标识当前packet在视频文件中的地址偏移
        ret = avcodec_send_packet(dec_ctx, packet);
        av_packet_unref(packet);
        if (ret == AVERROR(EAGAIN))
        {
            av_log(NULL, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, "
                                       "which is an API violation.\n");
        }
        
        if (ret != 0)
        {
            return ret;
        }
    }
}

