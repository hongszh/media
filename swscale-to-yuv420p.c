
/************************************************

---

## pkg-config查找依赖的库的include路径

$ pkg-config --cflags libavcodec
-I/usr/local/include

## pkg-config查找依赖的库的ld参数

$ pkg-config --libs libavcodec
-L/usr/local/lib -lavcode

$ pkg-config --libs libswscale
-L/usr/local/lib -lswscale


## ffmpeg的小程序通用编译方法
gcc -o trans trans.c -lswscale -lavcodec -lavformat -lavfilter -lavutil

---
 *************************************************/

#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libswscale/swscale.h>
#include <stdio.h>

int transform(const char *videoPath, const char *outputPath) {
    struct SwsContext *pSwsContext = NULL;

    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *codecCtx = NULL;
    AVCodec *avCodec = NULL;
    AVFrame *inFrame = NULL, *outFrame = NULL;
    AVPacket *readPacket = NULL;

    FILE *yuv_file = NULL;

    // 1.注册所有组件
    av_register_all();

    // 2.创建AVFormatContext结构体
    pFormatCtx = avformat_alloc_context();

    // 3.打开输入文件
    if (avformat_open_input(&pFormatCtx, videoPath, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open input stream.\n");
        goto stop;
    }
    // 4.获取stream info
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information.\n");
        goto stop;
    }

    //获取video track的index
    int videoIndex;
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            break;
        }
    if (videoIndex == -1) {
        fprintf(stderr, "Could not find a video stream.\n");
        goto stop;
    }
    // 5.查找decoder
    avCodec = avcodec_find_decoder(pFormatCtx->streams[videoIndex]->codecpar->codec_id);
    if (avCodec == NULL) {
        fprintf(stderr, "Could not find Codec.\n");
        goto stop;
    }

    // 6.分配打开解码器
    codecCtx = avcodec_alloc_context3(avCodec);
    avcodec_parameters_to_context(codecCtx, pFormatCtx->streams[videoIndex]->codecpar);
    codecCtx->thread_count = 1;

    if (avcodec_open2(codecCtx, avCodec, NULL) < 0) {
        fprintf(stderr, "Could not open codec.\n");
        goto stop;
    }

    int align = 1;
    inFrame = av_frame_alloc();
    outFrame = av_frame_alloc();
    // 输出frame的内存分配有几种方式，定义METHOD_X为1，使用METHOD_X方法
#define METHOD_1 0
#define METHOD_2 0
#define METHOD_3 1

#if METHOD_1
    // 方法一： av_malloc分配buffer，再用av_image_fill_arrays
    // 获取输出YUV420P最后一个参数align是1,如果使用16为对齐，得到的size比实际大小肯定要大
    int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                    codecCtx->width, codecCtx->height, align);
    uint8_t *outBuffer = (uint8_t *)av_malloc(bufferSize * sizeof(uint8_t));

    // 将outBuffer真实的内存空间分配给outFrame->data，完成指针指向内存块的操作
    // outFrame->data是一个指针数组，存放yuv分量的指针，大小是AV_NUM_DATA_POINTERS
    av_image_fill_arrays(outFrame->data, outFrame->linesize,
                         outBuffer, AV_PIX_FMT_YUV420P,
                         codecCtx->width, codecCtx->height, align);
#endif

#if METHOD_2
    // 使用av_image_alloc直接分配，代码少，简单易用
    // 分配大小为width*height以及像素格式pix_fmt的图像，并填充指针data和linesize
    if (av_image_alloc(outFrame->data, outFrame->linesize,
        codecCtx->width, codecCtx->height, AV_PIX_FMT_YUV420P, 1) < 0) {
        fprintf(stderr, "out frame: av_image_alloc failed!\n");
        goto stop;
    }
#endif

#if METHOD_3
    // 在调用此函数之前，必须在frame上设置以下字段：
    // -格式（视频的pix format，音频的sample format）
    // -视频的宽度width和高度height
    // -音频的采样数(比如AAC通常是1024)和channel_layout
    outFrame = av_frame_alloc();
    outFrame->format = AV_PIX_FMT_YUV420P;
    outFrame->width = codecCtx->width;
    outFrame->height = codecCtx->height;
    av_frame_get_buffer(outFrame, align);
#endif

    // 获取swscale context，输出格式为AV_PIX_FMT_YUV420P，采用快速双线性算法
    pSwsContext = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
                                 codecCtx->width, codecCtx->height, AV_PIX_FMT_YUV420P,
                                 SWS_FAST_BILINEAR, NULL, NULL, NULL);

    FILE *outputFile = fopen(outputPath, "wb");
    readPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    while (av_read_frame(pFormatCtx, readPacket) >= 0) {
        if (readPacket->stream_index == videoIndex) {
            if (avcodec_send_packet(codecCtx, readPacket) != 0) {
                return -1;
            }
            // inFrame是解码后的frame，作为sws_scale的输入frame
            while (avcodec_receive_frame(codecCtx, inFrame) == 0) {
                sws_scale(pSwsContext, (const uint8_t *const *)inFrame->data,
                          inFrame->linesize, 0, codecCtx->height, outFrame->data, outFrame->linesize);

                // 依次写入YUV420P的Y/U/V分量到文件
                fwrite(outFrame->data[0], (codecCtx->width) * (codecCtx->height), 1, outputFile);
                fwrite(outFrame->data[1], (codecCtx->width) * (codecCtx->height) / 4, 1, outputFile);
                fwrite(outFrame->data[2], (codecCtx->width) * (codecCtx->height) / 4, 1, outputFile);
            }
        }
        av_packet_unref(readPacket);
    }

    if (outputFile != NULL) {
        fclose(outputFile);
        outputFile = NULL;
    }

    sws_freeContext(pSwsContext);

stop:

    avcodec_close(codecCtx);
    avcodec_free_context(&codecCtx);
    av_free(inFrame);
    av_free(outFrame);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s input-file output-file.\n", argv[0]);
        return 0;
    }
    fprintf(stderr, " input: %s.\n", argv[1]);
    fprintf(stderr, "output: %s.\n", argv[2]);

    transform(argv[1], argv[2]);
}
