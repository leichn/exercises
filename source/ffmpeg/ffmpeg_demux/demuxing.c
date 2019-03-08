#include <libavformat/avformat.h>

int main (int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "usage: %s test.avi test.mpeg2video test.mp2\n", argv[0]);
        exit(1);
    }

    const char *input_fname = argv[1];
    const char *output_v_fname = argv[2];
    const char *output_a_fname = argv[3];
    FILE *video_dst_file = fopen(output_v_fname, "wb");
    FILE *audio_dst_file = fopen(output_a_fname, "wb");

    AVFormatContext *fmt_ctx = NULL;
    int ret = avformat_open_input(&fmt_ctx, input_fname, NULL, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open input file %s\n", input_fname);
        exit(1);
    }

    int video_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int audio_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    av_dump_format(fmt_ctx, 0, input_fname, 0);

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0)
    {
        if (pkt.stream_index == video_idx)
        {
            ret = fwrite(pkt.data, 1, pkt.size, video_dst_file);
            printf("Write video packet %x %3"PRId64" (size=%5d)\n", pkt.pos, pkt.pts, ret);
        }
        else if (pkt.stream_index == audio_idx) {
            ret = fwrite(pkt.data, 1, pkt.size, audio_dst_file);
            printf("Write audio packet %x %3"PRId64" (size=%5d)\n", pkt.pos, pkt.pts, ret);
        }
        av_packet_unref(&pkt);
    }

    printf("Demuxing succeeded.\n");

end:
    avformat_close_input(&fmt_ctx);
    fclose(video_dst_file);
    fclose(audio_dst_file);

    return 0;
}

