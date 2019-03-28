#include <stdbool.h>
#include "av_codec.h"

// 关于：avcodec_send_packet()与avcodec_receive_frame()
// 1. 按dts递增的顺序向解码器送入编码帧packet，解码器按pts递增的顺序输出原始帧frame，实际上解码器不关注输入packet的dts(错值都没关系)，它只管依次处理收到的packet，按需缓冲和解码
// 2. avcodec_receive_frame()输出frame时，会根据各种因素设置好frame->best_effort_timestamp(文档明确说明)，实测frame->pts也会被设置(通常直接拷贝自对应的packet.pts，文档未明确说明)
//    用户应确保avcodec_send_packet()发送的packet具有正确的pts，编码帧packet与原始帧frame间的对应关系通过pts确定
// 3. avcodec_receive_frame()输出frame时，frame->pkt_dts拷贝自当前avcodec_send_packet()发送的packet中的dts，如果当前packet为flush packet，解码器进入flush模式，
//    当前及剩余的frame->pkt_dts值总为AV_NOPTS_VALUE。因为解码器中有缓存帧，当前输出的frame并不是由当前输入的packet解码得到的，所以这个frame->pkt_dts没什么实际意义，可以不必关注
// 4. avcodec_send_packet()发送第一个 flush packet 会返回成功，后续的 flush packet 会返回AVERROR_EOF。
// 5. avcodec_send_packet()多次发送 flush packet 并不会导致解码器中缓存的帧丢失，使用avcodec_flush_buffers()可以立即丢掉解码器中缓存帧。因此播放完毕时应avcodec_send_packet(NULL)
//    来取完缓存的帧，而SEEK操作或切换流时应调用avcodec_flush_buffers()来直接丢弃缓存帧。
// 6. 解码器通常的冲洗方法：调用一次avcodec_send_packet(NULL)(返回成功)，然后不停调用avcodec_receive_frame()直到其返回AVERROR_EOF，取现所有缓存帧，avcodec_receive_frame()返回
//    AVERROR_EOF这一次是没有有效数据的，仅仅获取到一个结束标志。

// retrun 0:                got a frame success
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
                if (frame->pts == AV_NOPTS_VALUE)
                {
                    frame->pts = frame->best_effort_timestamp;
                    printf("set video pts %d\n", frame->pts);
                }
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
                if (frame->pts == AV_NOPTS_VALUE)
                {
                    frame->pts = frame->best_effort_timestamp;
                    printf("set audio pts %d\n", frame->pts);
                }
            }
        }

        if (ret >= 0)                   // 成功解码得到一个视频帧或一个音频帧，则返回
        {
            return ret;   
        }
        else if (ret == AVERROR_EOF)    // 解码器已冲洗，解码中所有帧已取出
        {
            avcodec_flush_buffers(dec_ctx);
            return ret;
        }
        else if (ret == AVERROR(EAGAIN))// 解码器需要喂数据
        {
            if (!(*new_packet))         // 本函数中已向解码器喂过数据，因此需要从文件读取新数据
            {
                //av_log(NULL, AV_LOG_INFO, "decoder need more packet\n");
                return ret;
            }
        }
        else                            // 错误
        {
            av_log(NULL, AV_LOG_ERROR, "decoder error %d\n", ret);
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


// 关于：avcodec_send_frame()与avcodec_receive_packet()
// 1. 按pts递增的顺序向编码器送入原始帧frame，编码器按dts递增的顺序输出编码帧packet，实际上编码器关注输入frame的pts不关注其dts，它只管依次处理收到的frame，按需缓冲和编码
// 2. avcodec_receive_packet()输出packet时，会设置packet.dts，从0开始，每次输出的packet的dts加1，这是视频层的dts，用户写输出前应将其转换为容器层的dts
// 3. avcodec_receive_packet()输出packet时，packet.pts拷贝自对应的frame.pts，这是视频层的pts，用户写输出前应将其转换为容器层的pts
// 4. avcodec_send_frame()发送NULL frame时，编码器进入flush模式
// 5. avcodec_send_frame()发送第一个NULL会返回成功，后续的NULL会返回AVERROR_EOF
// 6. avcodec_send_frame()多次发送NULL并不会导致编码器中缓存的帧丢失，使用avcodec_flush_buffers()可以立即丢掉编码器中缓存帧。因此编码完毕时应使用avcodec_send_frame(NULL)
//    来取完缓存的帧，而SEEK操作或切换流时应调用avcodec_flush_buffers()来直接丢弃缓存帧。
// 7. 编码器通常的冲洗方法：调用一次avcodec_send_frame(NULL)(返回成功)，然后不停调用avcodec_receive_packet()直到其返回AVERROR_EOF，取出所有缓存帧，avcodec_receive_packet()返回
//    AVERROR_EOF这一次是没有有效数据的，仅仅获取到一个结束标志。
// 8. 对音频来说，如果AV_CODEC_CAP_VARIABLE_FRAME_SIZE(在AVCodecContext.codec.capabilities变量中，只读)标志有效，表示编码器支持可变尺寸音频帧，送入编码器的音频帧可以包含
//    任意数量的采样点。如果此标志无效，则每一个音频帧的采样点数目(frame->nb_samples)必须等于编码器设定的音频帧尺寸(avctx->frame_size)，最后一帧除外，最后一帧音频帧采样点数
//    可以小于avctx->frame_size
int av_encode_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *packet)
{
    int ret = -1;
    
    // 第一次发送flush packet会返回成功，进入冲洗模式，可调用avcodec_receive_packet()
    // 将编码器中缓存的帧(可能不止一个)取出来
    // 后续再发送flush packet将返回AVERROR_EOF
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret == AVERROR_EOF)
    {
        //av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() encoder flushed\n");
    }
    else if (ret == AVERROR(EAGAIN))
    {
        //av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() need output read out\n");
    }
    else if (ret < 0)
    {
        //av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() error %d\n", ret);
        return ret;
    }

    ret = avcodec_receive_packet(enc_ctx, packet);
    if (ret == AVERROR_EOF)
    {
        av_log(NULL, AV_LOG_INFO, "avcodec_recieve_packet() encoder flushed\n");
    }
    else if (ret == AVERROR(EAGAIN))
    {
        //av_log(NULL, AV_LOG_INFO, "avcodec_recieve_packet() need more input\n");
    }
    
    return ret;
}

