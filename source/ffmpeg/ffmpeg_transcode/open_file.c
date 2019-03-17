#include "open_file.h"

int open_input_file(const char *filename, inout_ctx_t *ictx)
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

int open_output_file(const char *filename, const inout_ctx_t *ictx, inout_ctx_t *octx, AVAudioFifo*** afifo)
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

    AVAudioFifo **pp_audio_fifo = av_mallocz_array(ofmt_ctx->nb_streams, sizeof(AVAudioFifo *));
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
                // enc_ctx->codec->capabilities |= AV_CODEC_CAP_VARIABLE_FRAME_SIZE; // 只读标志

                // 初始化一个FIFO用于存储待编码的音频帧，初始化FIFO大小的1个采样点
                // av_audio_fifo_alloc()第二个参数是声道数，第三个参数是单个声道的采样点数
                // 采样格式及声道数在初始化FIFO时已设置，各处涉及FIFO大小的地方都是用的单个声道的采样点数
                pp_audio_fifo[i] = av_audio_fifo_alloc(enc_ctx->sample_fmt, enc_ctx->channels, 1);
                if (pp_audio_fifo == NULL)
                {
                    av_log(NULL, AV_LOG_ERROR, "Could not allocate FIFO\n");
                    return AVERROR(ENOMEM);
                }

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

            // 3.6 保存输出流contex
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
    (*afifo) = pp_audio_fifo;

    return 0;
}

