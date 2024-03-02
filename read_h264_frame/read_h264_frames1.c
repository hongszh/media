#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_CODE_PREFIX_LENGTH 3
#define START_CODE_LENGTH 4

int main(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Failed to open file\n");
        return -1;
    }

    // Allocate buffer for reading file
    int buffer_size = 1024 * 1024;
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);

    // Allocate buffer for storing frame data
    int frame_size = buffer_size;
    uint8_t *frame = (uint8_t *)malloc(frame_size);

    int frame_count = 0;
    int bytes_read = 0;
    int frame_start = 0;
    int frame_end = 0;
    int frame_length = 0;
    int start_code_prefix_found = 0;

    while ((bytes_read = fread(buffer, 1, buffer_size, fp)) > 0)
    {
        for (int i = 0; i < bytes_read; i++) {
            if (!start_code_prefix_found) {

                /*
                 * 这里用001来判断的好处是，当发现后面的四个字节是0001的时候，说明frame结
                 * 束，这时候buffer[i]的位置已经是下一个0001的0位置，下次循环进来的时候
                 * buffer指向的位置刚好是001，因为有i++运算，已经去掉了前导0
                 * (leading_zero_8bits)
                 *
                 * 如果是0001，那么经过i++，start_code_prefix_found的位置就是下下一个
                 * startcode的位置了。
                 */

                if (i < bytes_read - START_CODE_PREFIX_LENGTH) {
                    if (buffer[i] == 0x00 &&
                        buffer[i+1] == 0x00 &&
                        buffer[i+2] == 0x01) {
                        start_code_prefix_found = 1;
                        frame_start = i + START_CODE_PREFIX_LENGTH;
                    }
                }
            } else {
                if (i < bytes_read - START_CODE_LENGTH) {
                    if (buffer[i] == 0x00 &&
                        buffer[i+1] == 0x00 &&
                        buffer[i+2] == 0x00 &&
                        buffer[i+3] == 0x01) {

                        start_code_prefix_found = 0;
                        frame_end = i;
                        frame_length = frame_end - frame_start;

                        if (frame_length > frame_size) {
                            frame_size = frame_length;
                            frame = (uint8_t *)realloc(frame, frame_size);
                        }
                        memcpy(frame, buffer + frame_start, frame_length);
                        printf("Frame %d: %d bytes\n", frame_count++, frame_length + START_CODE_LENGTH);
                    }
                } else if (i == bytes_read-1) {
                    frame_length = bytes_read - frame_start;
                    memcpy(frame, buffer + frame_start, frame_length);
                    printf("Frame %d: %d bytes\n", frame_count++, frame_length + START_CODE_LENGTH);
                }
            }
        }
    }

    fclose(fp);
    free(buffer);
    free(frame);

    return 0;
}
