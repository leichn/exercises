#include <stdbool.h>
#include <libavformat/avformat.h>

/*
ffmpeg -i tnliny.flv -c:v copy -an tnliny_v.flv
ffmpeg -i tnliny.flv -c:a copy -vn tnliny_a.flv
./muxing tnliny_v.flv tnliny_a.flv tnliny_av.flv
*/

int main (int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "usage: %s test.h264 test.aac test.ts\n", argv[0]);
        exit(1);
    }

    const char *input_v_fname = argv[1];
    const char *input_a_fname = argv[2];
    const char *output_fname = argv[3];
    int ret = 0;

    // 1 打开两路输入
    // 1.1 打开第一路输入，并找到一路视频流
    AVFormatContext *v_ifmt_ctx = NULL;
    ret = avformat_open_input(&v_ifmt_ctx, input_v_fname, NULL, NULL);
    ret = avformat_find_stream_info(v_ifmt_ctx, NULL);
    int video_idx = av_find_best_stream(v_ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream *in_v_stream = v_ifmt_ctx->streams[video_idx];
    // 1.2 打开第二路输入，并找到一路音频流
    AVFormatContext *a_ifmt_ctx = NULL;
    ret = avformat_open_input(&a_ifmt_ctx, input_a_fname, NULL, NULL);
    ret = avformat_find_stream_info(a_ifmt_ctx, NULL);
    int audio_idx = av_find_best_stream(a_ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    AVStream *in_a_stream = a_ifmt_ctx->streams[audio_idx];
    
    av_dump_format(v_ifmt_ctx, 0, input_v_fname, 0);
    av_dump_format(a_ifmt_ctx, 1, input_a_fname, 0);

    if (video_idx < 0 || audio_idx < 0)
    {
        printf("find stream failed: %d %d\n", video_idx, audio_idx);
        return -1;
    }

    
    // 2 打开输出，并向输出中添加两路流，一路用于存储视频，一路用于存储音频
    AVFormatContext *ofmt_ctx = NULL;
    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_fname);
    
    AVStream *out_v_stream = avformat_new_stream(ofmt_ctx, NULL);
    ret = avcodec_parameters_copy(out_v_stream->codecpar, in_v_stream->codecpar);

    AVStream *out_a_stream = avformat_new_stream(ofmt_ctx, NULL);
    ret = avcodec_parameters_copy(out_a_stream->codecpar, in_a_stream->codecpar);
    
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) // TODO: 研究AVFMT_NOFILE标志
    { 
        ret = avio_open(&ofmt_ctx->pb, output_fname, AVIO_FLAG_WRITE);
    }

    av_dump_format(ofmt_ctx, 0, output_fname, 1);

    // 3 写输入文件头
    ret = avformat_write_header(ofmt_ctx, NULL);

    AVPacket vpkt;
    av_init_packet(&vpkt);
    vpkt.data = NULL;
    vpkt.size = 0;

    AVPacket apkt;
    av_init_packet(&apkt);
    apkt.data = NULL;
    apkt.size = 0;

    AVPacket *p_pkt = NULL;

    int64_t vdts = 0;
    int64_t adts = 0;
    
    bool video_finished = false;
    bool audio_finished = false;
    bool v_or_a = false;

    // 4 从两路输入依次取得packet，交织存入输出中
    printf("V/A\tPTS\tDTS\tSIZE\n");
    while (1)
    {
        if (vpkt.data == NULL && (!video_finished))
        {
            while (1)   // 取出一个video packet，退出循环
            {
                ret = av_read_frame(v_ifmt_ctx, &vpkt);
                if ((ret == AVERROR_EOF) || avio_feof(v_ifmt_ctx->pb))
                {
                    printf("video finished\n");
                    video_finished = true;
                    vdts = AV_NOPTS_VALUE;
                    break;
                }
                else if (ret < 0)
                {
                    printf("video read error\n");
                    goto end;
                }
                
                if (vpkt.stream_index == video_idx)
                {
                    // 更新packet中的pts和dts。关于AVStream.time_base的说明：
                    // 输入：输入流中含有time_base，在avformat_find_stream_info()中可取到每个流中的time_base
                    // 输出：avformat_write_header()会根据输出的封装格式确定每个流的time_base并写入文件中
                    // AVPacket.pts和AVPacket.dts的单位是AVStream.time_base，不同的封装格式其AVStream.time_base不同
                    // 所以输出文件中，每个packet需要根据输出封装格式重新计算pts和dts
                    av_packet_rescale_ts(&vpkt, in_v_stream->time_base, out_v_stream->time_base);
                    vpkt.pos = -1;      // 让muxer根据重新将packet在输出容器中排序
                    vpkt.stream_index = 0;
                    vdts = vpkt.dts;
                    break;
                }
                av_packet_unref(&vpkt);
            }

        }
        
        if (apkt.data == NULL && (!audio_finished))
        {
            while (1)   // 取出一个audio packet，退出循环
            {
                ret = av_read_frame(a_ifmt_ctx, &apkt);
                if ((ret == AVERROR_EOF) || avio_feof(a_ifmt_ctx->pb))
                {
                    printf("audio finished\n");
                    audio_finished = true;
                    adts = AV_NOPTS_VALUE;
                    break;
                }
                else if (ret < 0)
                {
                    printf("audio read error\n");
                    goto end;
                }
                
                if (apkt.stream_index == audio_idx)
                {
                    av_packet_rescale_ts(&apkt, in_a_stream->time_base, out_a_stream->time_base);
                    apkt.pos = -1;
                    apkt.stream_index = 1;
                    adts = apkt.dts;
                    break;
                }
                av_packet_unref(&apkt);
            }

        }

        if (video_finished && audio_finished)
        {
            printf("all read finished. flushing queue.\n");
            //av_interleaved_write_frame(ofmt_ctx, NULL); // 冲洗交织队列
            break;
        }
        else                            // 音频或视频未读完
        {
            if (video_finished)         // 视频读完，音频未读完
            {
                v_or_a = false;
            }
            else if (audio_finished)    // 音频读完，视频未读完
            {
                v_or_a = true;
            }
            else                        // 音频视频都未读完
            {
                // video pakect is before audio packet?
                ret = av_compare_ts(vdts, out_v_stream->time_base, adts, out_a_stream->time_base);
                v_or_a = (ret <= 0);
            }

            
            p_pkt = v_or_a ? &vpkt : &apkt;
            printf("%s\t%3"PRId64"\t%3"PRId64"\t%-5d\n", v_or_a ? "vp" : "ap", 
                   p_pkt->pts, p_pkt->dts, p_pkt->size);
            //ret = av_interleaved_write_frame(ofmt_ctx, p_pkt);
            ret = av_write_frame(ofmt_ctx, p_pkt);
            
            if (p_pkt->data != NULL)
            {
                av_packet_unref(p_pkt);
            }
        }
    }

    // 5 写输出文件尾
    av_write_trailer(ofmt_ctx);

    printf("Muxing succeeded.\n");

end:
    avformat_close_input(&v_ifmt_ctx);
    avformat_close_input(&a_ifmt_ctx);
    avformat_free_context(ofmt_ctx);
    return 0;
}

