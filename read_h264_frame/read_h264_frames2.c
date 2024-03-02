#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define START_CODE_LENGTH 4

int main(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        return 1;
    }

    uint8_t start_code[START_CODE_LENGTH] = {0x00, 0x00, 0x00, 0x01};
    uint8_t *frame_buffer = NULL;
    size_t frame_buffer_size = 0;
    int frame_idx = 0;

    while (!feof(fp)) {
        // Read the next byte from the file
        uint8_t byte;
        fread(&byte, 1, 1, fp);

        // Check if the byte matches the start code prefix
        if (byte == start_code[0]) {
            fread(&byte, 1, 1, fp);
            if (byte == start_code[1]) {
                fread(&byte, 1, 1, fp);
                if (byte == start_code[2]) {
                    // Read the next byte to determine the type of start code
                    fread(&byte, 1, 1, fp);
                    if (byte == 0x01) {
                        // This is a sequence header start code
                        // Do nothing
                    }

                    // Read the rest of the frame into the frame buffer
                    size_t buffer_size = 0;
                    while (!feof(fp)) {
                        fread(&byte, 1, 1, fp);
                        buffer_size++;
                        if (byte == 0x00) {
                            fread(&byte, 1, 1, fp);
                            buffer_size++;
                            if (byte == 0x00) {
                                fread(&byte, 1, 1, fp);
                                buffer_size++;
                                if (byte == 0x00) {
                                    fread(&byte, 1, 1, fp);
                                    buffer_size++;
                                    if (byte == 0x01) {
                                        // This is the start of a new frame
                                        fseek(fp, -4, SEEK_CUR);
                                        buffer_size -= 4;
                                        break;
                                    } else {
                                        fseek(fp, -3, SEEK_CUR);
                                        buffer_size -= 3;
                                    }
                                } else {
                                    fseek(fp, -2, SEEK_CUR);
                                    buffer_size -= 2;
                                }
                            } else {
                                fseek(fp, -1, SEEK_CUR);
                                buffer_size--;
                            }
                        }
                    }

                    // Allocate memory for the frame buffer
                    frame_buffer = (uint8_t *)malloc(buffer_size);
                    if (!frame_buffer) {
                        fprintf(stderr, "Failed to allocate memory for frame buffer\n");
                        return 1;
                    }
                    frame_buffer_size = buffer_size;

                    fprintf(stderr, "frame %d: %zu bytes\n", frame_idx++, frame_buffer_size + START_CODE_LENGTH);

                    // Read the frame into the frame buffer
                    fseek(fp, -buffer_size, SEEK_CUR);
                    fread(frame_buffer, 1, buffer_size, fp);
                } else {
                    fseek(fp, -2, SEEK_CUR);
                }
            } else {
                fseek(fp, -1, SEEK_CUR);
            }
        }
    }

    fclose(fp);
    return 0;
}
