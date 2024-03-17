#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

int main(int argc, char *argv[])
{
    AVCodecContext *ctx;
    const AVCodec *codec;
    AVFrame *frame;
    AVPacket *pkt;
    FILE *fp_in, *fp_out;
    char *in_file, *out_file;
    uint8_t *buf;

    int video_size, buf_size;
    int opt, width, height;
    int frame_count = 0;
    int ret;


    if (argc < 6) {
        fprintf(stderr, "Usage: %s -i <input_file> -s <width>x<height> -o <output_file> \n", argv[0]);
        return 1;
    }

    while ((opt = getopt(argc, argv, "s:i:o:")) != -1) {
        switch (opt) {
            case 's':
                if (sscanf(optarg, "%dx%d", &width, &height) != 2) {
                    fprintf(stderr, "Invalid size argument: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'i':
                in_file = optarg;
                break;

            case 'o':
                out_file = optarg;
                break;

            default:
                fprintf(stderr, "Usage: %s [-s size] [-i input] [-o output]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    printf("Size: %dx%d\n", width, height);
    printf("Input file: %s\n", in_file);
    printf("Output file: %s\n", out_file);

    // open input file
    fp_in = fopen(in_file, "rb");
    if (!fp_in) {
        fprintf(stderr, "Failed to open input file '%s'\n", in_file);
        goto end;
    }

    // open output file
    fp_out = fopen(out_file, "wb");
    if (!fp_out) {
        fprintf(stderr, "Failed to open output file '%s'\n", out_file);
        goto end;
    }

    // init AVCodec AVCodecContext
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Failed to find H.264 encoder\n");
        goto end;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        fprintf(stderr, "Failed to allocate codec context\n");
        goto end;
    }

    ctx->width = width;
    ctx->height = height;
    ctx->time_base = (AVRational){1, 25};
    ctx->framerate = (AVRational){25, 1};
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->bit_rate = 4000000;

    // open encoder
    ret = avcodec_open2(ctx, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Failed to open codec: %s\n", av_err2str(ret));
        goto end;
    }

    // alloc AVFrame AVPacket
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Failed to allocate frame\n");
        goto end;
    }

    frame->format = ctx->pix_fmt;
    frame->width  = ctx->width;
    frame->height = ctx->height;

    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Failed to allocate frame buffer: %s\n", av_err2str(ret));
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Failed to allocate packet\n");
        goto end;
    }

    // alloc buffer
    buf_size = av_image_get_buffer_size(ctx->pix_fmt, ctx->width, ctx->height, 1);
    buf = av_malloc(buf_size);
    if (!buf) {
        fprintf(stderr, "Failed to allocate buffer\n");
        goto end;
    }

    // read YUV and encode
    while (1) {
        ret = fread(buf, 1, buf_size, fp_in);
        if (ret < buf_size) {
            break;
        }

        // fill AVFrame
        ret = av_image_fill_arrays(frame->data, frame->linesize, buf,
                                   ctx->pix_fmt, ctx->width, ctx->height, 1);
        if (ret < 0) {
            fprintf(stderr, "Failed to fill frame: %s\n", av_err2str(ret));
            goto end;
        }

        frame->pts = frame_count++;

        // send AVPacket
        ret = avcodec_send_frame(ctx, frame);
        if (ret < 0) {
            fprintf(stderr, "Failed to send frame: %s\n", av_err2str(ret));
            goto end;
        }

        // receive AVPacket
        while (ret >= 0) {
            ret = avcodec_receive_packet(ctx, pkt);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                fprintf(stderr, "Failed to receive packet: %s\n", av_err2str(ret));
                goto end;
            }

            fprintf(stderr, "receive packet: %ld\n", pkt->pts);

            // write output file
            fwrite(pkt->data, 1, pkt->size, fp_out);
            av_packet_unref(pkt);
        }
    }

    // send AVPacket
    ret = avcodec_send_frame(ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Failed to send frame: %s\n", av_err2str(ret));
        goto end;
    }

    // receive AVPacket
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        } else if (ret < 0) {
            fprintf(stderr, "Failed to receive packet: %s\n", av_err2str(ret));
            goto end;
        }

        // write output file
        fwrite(pkt->data, 1, pkt->size, fp_out);
        av_packet_unref(pkt);
    }

end:
    if (fp_in) {
        fclose(fp_in);
    }

    if (fp_out) {
        fclose(fp_out);
    }

    if (ctx) {
        avcodec_free_context(&ctx);
    }

    if (frame) {
        av_frame_free(&frame);
    }

    if (pkt) {
        av_packet_free(&pkt);
    }

    if (buf) {
        av_free(buf);
    }

    return ret < 0;
}
