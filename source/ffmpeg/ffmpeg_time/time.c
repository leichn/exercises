#include <libavformat/avformat.h>

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s test.ts\n", argv[0]);
        exit(1);
    }

    const char *input_fname = argv[1];

    AVFormatContext *fmt_ctx = NULL;
    int ret = avformat_open_input(&fmt_ctx, input_fname, NULL, NULL);
    ret = avformat_find_stream_info(fmt_ctx, NULL);

    int vst_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int ast_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (vst_idx < 0 || ast_idx < 0)
    {
        printf("find stream failed: %d %d\n", vst_idx, ast_idx);
        return -1;
    }

    av_dump_format(fmt_ctx, 0, input_fname, 0);

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

	int vpkt_idx = 0;
    AVRational tb_q = fmt_ctx->streams[vst_idx]->time_base;
    double tb_d = av_q2d(tb_q);

	printf("N\tV/A\tPOS\tPTS\tDTS\tSIZE\n");
    while (av_read_frame(fmt_ctx, &pkt) >= 0)
    {
        if (pkt.stream_index == vst_idx)
        {
            printf("%d\tvp\t%x\t%lld\t%lld\t(size=%5d)\n", ++vpkt_idx, pkt.pos, pkt.pts, pkt.dts, pkt.size);
        }
        else if (pkt.stream_index == ast_idx)
		{
            printf("%d\tap\t%x\t%3"PRId64"\t%3"PRId64"\t(size=%5d)\n", ++vpkt_idx, pkt.pos, pkt.pts, pkt.dts, pkt.size);
        }
        av_packet_unref(&pkt);
    }

    printf("Demuxing succeeded.\n");

end:
    avformat_close_input(&fmt_ctx);

    return 0;
}

