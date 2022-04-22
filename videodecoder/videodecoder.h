#ifndef __VIDEODECODER_H__
#define __VIDEODECODER_H__

#include <stdint.h>

#ifdef __arm__

#ifdef USB_UVC
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#else
#define IMAGE_WIDTH 720
#define IMAGE_HEIGHT 576
#endif

#define CANVAS_WIDTH 736
#define CANVAS_HEIGHT 576
#endif

void decode_init();
void decode_frame(char* data, uint32_t frame_number);

#endif
