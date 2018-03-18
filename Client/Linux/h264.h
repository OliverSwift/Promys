#ifndef _H262_H_
#define _H262_H_

#include <sys/types.h>

#define OUTPUT_WIDTH   1920
#define OUTPUT_HEIGHT  1080

void h264_init(size_t input_width, size_t input_height, size_t stride);

unsigned char * h264_encode(const unsigned char *image_data, size_t * packet_size);

void h264_close();

#endif // _H262_H_
