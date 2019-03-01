/*
 * Copyright (c) 2010 Nicolas George
 * Copyright (c) 2011 Stefano Sabatini
 * Copyright (c) 2014 Andrey Utkin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * API example for demuxing, decoding, filtering, encoding and muxing
 * @example transcoding.c
 */

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>

#include "av_filter.h"
#include "av_codec.h"

typedef struct {
    AVFormatContext* fmt_ctx;
    AVCodecContext** codec_ctx; // AVCodecContext* codec_ctx[];
    int              video_idx;
    int              frame_rate;
}   inout_ctx_t;

static void get_filter_ivfmt(const inout_ctx_t *ictx, int stream_idx, filter_ivfmt_t *ivfmt)
{
    ivfmt->width = ictx->codec_ctx[stream_idx]->width;
    ivfmt->height = ictx->codec_ctx[stream_idx]->height;
    ivfmt->pix_fmt = ictx->codec_ctx[stream_idx]->pix_fmt;
    ivfmt->sar = ictx->codec_ctx[stream_idx]->sample_aspect_ratio;
    ivfmt->time_base = ictx->fmt_ctx->streams[stream_idx]->time_base;
    ivfmt->frame_rate = ictx->fmt_ctx->streams[stream_idx]->avg_frame_rate;
    av_log(NULL, AV_LOG_INFO, "get video format: "
            "%dx%d, pix_fmt %d, SAR %d/%d, tb {%d, %d}, rate {%d, %d}\n",
            ivfmt->width, ivfmt->height, ivfmt->pix_fmt,
            ivfmt->sar.num, ivfmt->sar.den,
            ivfmt->time_base.num, ivfmt->time_base.den,
            ivfmt->frame_rate.num, ivfmt->frame_rate.den);
}

static void get_filter_ovfmt(const inout_ctx_t *octx, int stream_idx, filter_ovfmt_t *ovfmt)
{
    enum AVPixelFormat *pix_fmts;

}

static void get_filter_iafmt(const inout_ctx_t *ictx, int stream_idx, filter_iafmt_t *iafmt)
{

}

static void get_filter_oafmt(const inout_ctx_t *octx, int stream_idx, filter_oafmt_t *oafmt)
{

}


static int open_input_file(const char *filename, inout_ctx_t *ictx)
{
    int ret;
    unsigned int i;

    AVFormatContext *ifmt_ctx = NULL;
    // 1. 打开视频文件：读取文件头，将文件格式信息存储在ifmt_ctx中
    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    ictx->fmt_ctx = ifmt_ctx;

    // 2. 搜索流信息：读取一段视频文件数据，尝试解码，将取到的流信息填入ifmt_ctx.streams
    //    ifmt_ctx.streams是一个指针数组，数组大小是ifmt_ctx.nb_streams
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    // 每路音频流/视频流一个AVCodecContext
    AVCodecContext **pp_dec_ctx = av_mallocz_array(ifmt_ctx->nb_streams, sizeof(AVCodecContext *));
    if (!pp_dec_ctx)
    {
        return AVERROR(ENOMEM);
    }

    // 3. 将输入文件中各流对应的AVCodecContext存入数组
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        // 3.1 获取解码器AVCodec
        AVStream *stream = ifmt_ctx->streams[i];
        AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext *codec_ctx;
        if (!dec)
        {
            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
            return AVERROR_DECODER_NOT_FOUND;
        }
        // 3.2 AVCodecContext初始化：分配结构体，使用AVCodec初始化AVCodecContext相应成员为默认值
        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx)
        {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
            return AVERROR(ENOMEM);
        }
        // 3.3 AVCodecContext初始化：使用codec参数codecpar初始化AVCodecContext相应成员
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                    "for stream #%u\n", i);
            return ret;
        }
        /* Reencode video & audio and remux subtitles etc. */
        // 音频流视频流需要重新编码，字幕流只需要重新封装
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
                || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
                codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
            /* Open decoder */
            // 3.4 AVCodecContext初始化：使用codec参数codecpar初始化AVCodecContext，初始化完成
            ret = avcodec_open2(codec_ctx, dec, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
        pp_dec_ctx[i] = codec_ctx;
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);

    ictx->fmt_ctx = ifmt_ctx;
    ictx->codec_ctx = pp_dec_ctx;

    return 0;
}

static int open_output_file(const char *filename, const inout_ctx_t *ictx, inout_ctx_t *octx)
{
    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder;
    int ret;
    unsigned int i;

    // 1. 分配输出ctx
    AVFormatContext *ofmt_ctx = NULL;
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }

    // 每路音频流/视频流一个AVCodecContext
    AVCodecContext **pp_enc_ctx = av_mallocz_array(ofmt_ctx->nb_streams, sizeof(AVCodecContext *));
    if (!pp_enc_ctx)
    {
        return AVERROR(ENOMEM);
    }

    AVFormatContext *ifmt_ctx = ictx->fmt_ctx;
    for (i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        // 2. 将一个新流(out_stream)添加到输出文件(ofmt_ctx)
        AVStream *out_stream = NULL;
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream)
        {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }

        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecContext *dec_ctx = ictx->codec_ctx[i];

        // 3. 构建AVCodecContext
        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO ||
                dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)          // 音频流或视频流
        {
            // 3.1 查找编码器AVCodec，本例使用与解码器相同的编码器
            AVCodec *encoder = avcodec_find_encoder(dec_ctx->codec_id);
            if (!encoder)
            {
                av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
                return AVERROR_INVALIDDATA;
            }
            // 3.2 AVCodecContext初始化：分配结构体，使用AVCodec初始化AVCodecContext相应成员为默认值
            AVCodecContext *enc_ctx = avcodec_alloc_context3(encoder);
            if (!enc_ctx)
            {
                av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
                return AVERROR(ENOMEM);
            }

            // 3.3 AVCodecContext初始化：配置图像/声音相关属性
            /* In this example, we transcode to same properties (picture size,
             * sample rate etc.). These properties can be changed for output
             * streams easily using filters */
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                enc_ctx->height = dec_ctx->height;              // 图像高
                enc_ctx->width = dec_ctx->width;                // 图像宽
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio; // 采样宽高比：像素宽/像素高
                /* take first format from list of supported formats */
                if (encoder->pix_fmts)  // 编码器支持的像素格式列表
                {
                    enc_ctx->pix_fmt = encoder->pix_fmts[0];    // 编码器采用所支持的第一种像素格式
                }
                else
                {
                    enc_ctx->pix_fmt = dec_ctx->pix_fmt;        // 编码器采用解码器的像素格式
                }
                /* video time_base can be set to whatever is handy and supported by encoder */
                enc_ctx->time_base = av_inv_q(dec_ctx->framerate);  // 时基：解码器帧率取倒数
            }
            else
            {
                enc_ctx->sample_rate = dec_ctx->sample_rate;    // 采样率
                enc_ctx->channel_layout = dec_ctx->channel_layout; // 声道布局
                enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout); // 声道数量
                /* take first format from list of supported formats */
                enc_ctx->sample_fmt = encoder->sample_fmts[0];  // 编码器采用所支持的第一种采样格式
                enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate}; // 时基：编码器采样率取倒数
            }

            // TODO: 这个标志还不懂，以后研究
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            {
                enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }

            // 3.4 AVCodecContext初始化：使用AVCodec初始化AVCodecContext，初始化完成
            /* Third parameter can be used to pass settings to encoder */
            ret = avcodec_open2(enc_ctx, encoder, NULL);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
                return ret;
            }
            // 3.5 设置输出流codecpar
            ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
                return ret;
            }

            // 3.6 设置输出流time_base
            out_stream->time_base = enc_ctx->time_base;
            // 3.7 保存输出流contex
            pp_enc_ctx[i] = enc_ctx;
        } 
        else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN)
        {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
            return AVERROR_INVALIDDATA;
        }
        else
        {    // 字幕流等
            /* if this stream must be remuxed */
            // 3. 将当前输入流中的参数拷贝到输出流中
            ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            if (ret < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
                return ret;
            }
            out_stream->time_base = in_stream->time_base;
        }

    }
    av_dump_format(ofmt_ctx, 0, filename, 1);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) // TODO: 研究AVFMT_NOFILE标志
    { 
        // 4. 创建并初始化一个AVIOContext，用以访问URL(out_filename)指定的资源
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }

    // 5. 写输出文件头
    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }

    octx->fmt_ctx = ofmt_ctx;
    octx->codec_ctx = pp_enc_ctx;

    return 0;
}

// 为每个音频流/视频流使用空滤镜，滤镜图中将buffer滤镜和buffersink滤镜直接相连
// 目的是：通过视频buffersink滤镜将视频流输出像素格式转换为编码器采用的像素格式
//         通过音频abuffersink滤镜将音频流输出声道布局转换为编码器采用的声道布局
//         为下一步的编码操作作好准备
static int init_filters(const inout_ctx_t *ictx, const inout_ctx_t *octx, filter_ctx_t *fctxs[])
{
    int nb_streams = ictx->fmt_ctx->nb_streams;
    fctxs = av_malloc_array(nb_streams, sizeof(*(*fctxs)));
    if (!fctxs)
    {
        return AVERROR(ENOMEM);
    }

    filter_ivfmt_t ivfmt;
    filter_ovfmt_t ovfmt;
    filter_iafmt_t iafmt;
    filter_oafmt_t oafmt;
    enum AVMediaType codec_type;
    for (int i = 0; i < nb_streams; i++)
    {
        codec_type = ictx->fmt_ctx->streams[i]->codecpar->codec_type;

        int ret = 0;
        if (codec_type == AVMEDIA_TYPE_VIDEO)
        {
            get_filter_ivfmt(ictx, i, &ivfmt);
            get_filter_ovfmt(octx, i, &ovfmt);
            ret = init_video_filters("null", &ivfmt, &ovfmt, fctxs[i]);
        }
        else if (codec_type == AVMEDIA_TYPE_AUDIO)
        {
            get_filter_iafmt(ictx, i, &iafmt);
            get_filter_oafmt(octx, i, &oafmt);
            ret = init_audio_filters("anull", &iafmt, &oafmt, fctxs[i]);
        }
        else
        {
            fctxs[i] = NULL;
        }

        if (ret < 0)
        {
            return ret;
        }
    }
    return 0;
}

#if 0
static int flush_encoder(unsigned int stream_index)
{
    int ret;
    int got_frame;

    if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
                AV_CODEC_CAP_DELAY))
        return 0;

    while (1) {
        av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
        ret = encode_write_frame(NULL, stream_index, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame)
            return 0;
    }
    return ret;
}
#endif

int main(int argc, char **argv)
{
    int ret;
    AVPacket ipacket = { .data = NULL, .size = 0 };
    AVPacket opacket = { .data = NULL, .size = 0 };
    AVFrame *frame = NULL;
    AVFrame *frame_flt = NULL;
    enum AVMediaType type;
    unsigned int stream_index;
    unsigned int i;
    int got_frame;

    if (argc != 3) {
        av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    // 1. 初始化：打开输入，打开输出，初始化滤镜
    inout_ctx_t ictx;
    if ((ret = open_input_file(argv[1], &ictx)) < 0)
    {
        goto end;
    }
    inout_ctx_t octx;
    if ((ret = open_output_file(argv[2], &ictx, &octx)) < 0)
    {
        goto end;
    }
    filter_ctx_t *fctxs;
    if ((ret = init_filters(&ictx, &octx, &fctxs)) < 0)
    {
        goto end;
    }

    /* read all packets */
    while (1) {
        // 2. 从输入文件读取packet
        ret = av_read_frame(ictx.fmt_ctx, &ipacket);
        if (ret < 0)
        {
            if ((ret == AVERROR_EOF) || avio_feof(ictx.fmt_ctx->pb))
            {
                // 输入文件已读完，发送NULL packet以冲洗(flush)解码器，否则解码器中缓存的帧取不出来
                
            }
            else
            {
                goto end;
            }
        }

        int stream_index = ipacket.stream_index;
        enum AVMediaType codec_type = ictx.fmt_ctx->streams[stream_index]->codecpar->codec_type;
        av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n", stream_index);

        if ((codec_type == AVMEDIA_TYPE_VIDEO) || (codec_type == AVMEDIA_TYPE_AUDIO))
        {
            while (1)
            {
                ret = av_decode_frame(ictx.codec_ctx[stream_index], &ipacket, frame);
                if (ret == AVERROR(EAGAIN))     // 需要读取新的packet喂给解码器
                {
                    break;
                }
                else if (ret == AVERROR_EOF)
                {
                    break;
                }

                ret = filtering_frame(&fctxs[stream_index], frame, frame_flt);
                if (ret < 0)
                {
                    goto end;
                }

                ret = av_encode_frame(octx.codec_ctx[stream_index], frame_flt, &opacket);
                if (ret == AVERROR(EAGAIN))     // 需要获取新的frame喂给编码器
                {
                    continue;
                }
                else if (ret == AVERROR_EOF)
                {
                    break;
                }

                // 2. AVPacket.pts和AVPacket.dts的单位是AVStream.time_base，不同的封装格式其AVStream.time_base不同
                //    所以输出文件中，每个packet需要根据输出封装格式重新计算pts和dts
                /* prepare packet for muxing */
                opacket.stream_index = stream_index;
                av_packet_rescale_ts(&opacket,
                                     octx.codec_ctx[stream_index]->time_base,
                                     octx.fmt_ctx->streams[stream_index]->time_base);
                
                av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
                /* mux encoded frame */
                // 3. 将编码后的packet写入输出媒体文件
                ret = av_interleaved_write_frame(octx.fmt_ctx, &opacket);

            }
        }
        else
        {
            /* remux this frame without reencoding */
            // AVPacket.pts和AVPacket.dts的单位是AVStream.time_base，不同的封装格式其AVStream.time_base不同
            // 所以输出文件中，每个packet需要根据输出封装格式重新计算pts和dts
            av_packet_rescale_ts(&ipacket,
                                 octx.codec_ctx[stream_index]->time_base,
                                 octx.fmt_ctx->streams[stream_index]->time_base);

            ret = av_interleaved_write_frame(octx.fmt_ctx, &ipacket);
            if (ret < 0)
            {
                goto end;
            }

        }

        av_packet_unref(&ipacket);
        av_packet_unref(&opacket);
    }

#if 0
    /* flush filters and encoders */
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        /* flush filter */
        if (!filter_ctx[i].filter_graph)
            continue;
        ret = filter_encode_write_frame(NULL, i);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing filter failed\n");
            goto end;
        }

        /* flush encoder */
        ret = flush_encoder(i);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
            goto end;
        }
    }
#endif

    av_write_trailer(octx.fmt_ctx);

end:
    if (ret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
    }

    av_packet_unref(&ipacket);
    av_packet_unref(&opacket);
    
    av_frame_free(&frame);
    for (i = 0; i < ictx.fmt_ctx->nb_streams; i++)
    {
        avcodec_free_context(&ictx.codec_ctx[i]);
        avcodec_free_context(&octx.codec_ctx[i]);
        av_free(ictx.codec_ctx);
        av_free(octx.codec_ctx);
        deinit_filters(&fctxs[i]);
    }

    av_free(fctxs);

    avformat_close_input(&ictx.fmt_ctx);
    if (octx.fmt_ctx && !(octx.fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&octx.fmt_ctx->pb);
    }
    avformat_free_context(octx.fmt_ctx);

    return ret ? 1 : 0;
}
