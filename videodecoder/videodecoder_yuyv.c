#ifdef VIDEODECODER_YUYV

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "videodecoder.h"

char *buffer0 = NULL;
char *buffer1 = NULL;

char *uyvybuffer = NULL;

void decode_init() {
    uyvybuffer = (char*)malloc(CANVAS_WIDTH*CANVAS_HEIGHT * (2+1));
    memset(uyvybuffer, 0x0, CANVAS_WIDTH*CANVAS_HEIGHT * (2+1));
}

char *decode_frame(char* data, uint32_t frame_number) {
    if(data == NULL)    return;

    for(int y = 1; y < IMAGE_HEIGHT; y++) {
        memcpy(uyvybuffer + (y * CANVAS_WIDTH * 2), data  + (y * IMAGE_WIDTH * 2), IMAGE_WIDTH * 2);
    }

    return uyvybuffer;
}

#endif
