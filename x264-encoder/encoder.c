#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <x264.h>

int main(int argc, char **argv)
{
    int opt;
    int width = 0, height = 0;

    if (argc < 3) {
        printf("Usage: %s input.yuv output.h264\n", argv[0]);
        return -1;
    }

    char *input_file = argv[1];
    char *output_file = argv[2];

    while ((opt = getopt(argc, argv, "v:")) != -1) {
        switch (opt) {
            case 'v':
                /* optarg是一个全局变量，用于存储`getopt()`函数返回的选项参数的值。 */
                sscanf(optarg, "%dx%d", &width, &height);
                break;
            default:
                fprintf(stderr, "Usage: %s -v video_size\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    printf("Video size: %dx%d\n", width, height);

    x264_param_t param;
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    param.i_width = width;
    param.i_height = height;
    param.i_fps_num = 25;
    param.i_fps_den = 1;
    param.i_csp = X264_CSP_I420;

    x264_t *encoder = x264_encoder_open(&param);

    x264_picture_t pic_in, pic_out;
    x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);
    pic_in.i_pts = 0;

    FILE *input_fp = fopen(input_file, "rb");
    if (!input_fp) {
        printf("Failed to open input file: %s\n", input_file);
        return -1;
    }

    FILE *output_fp = fopen(output_file, "wb");
    if (!output_fp) {
        printf("Failed to open output file: %s\n", output_file);
        fclose(input_fp);
        return -1;
    }

    int frame_size = width * height * 3 / 2;
    uint8_t *frame_buf = malloc(frame_size);

    int i = 0;
    while (fread(frame_buf, 1, frame_size, input_fp) == frame_size) {
        memcpy(pic_in.img.plane[0], frame_buf, width * height);
        memcpy(pic_in.img.plane[1], frame_buf + width * height, width * height / 4);
        memcpy(pic_in.img.plane[2], frame_buf + width * height * 5 / 4, width * height / 4);

        x264_nal_t *nal;
        int i_nal;
        int ret = x264_encoder_encode(encoder, &nal, &i_nal, &pic_in, &pic_out);
        if (ret < 0) {
            printf("x264_encoder_encode failed\n");
            break;
        }

        for (i = 0; i < i_nal; i++) {
            fwrite(nal[i].p_payload, 1, nal[i].i_payload, output_fp);
        }
    }

    x264_encoder_close(encoder);
    x264_picture_clean(&pic_in);
    free(frame_buf);
    fclose(input_fp);
    fclose(output_fp);

    return 0;
}

#if 0

build:
gcc -o encoder encoder.c -lx264 -L/usr/local/lib

example:
./encoder 256x144-yuv420p.yuv 256x144.h264 -v 256x144

play h264:
ffplay 256x144.h264

#endif