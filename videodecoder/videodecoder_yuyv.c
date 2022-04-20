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

void decode_frame(char* data, uint32_t frame_number) {
    if(frame_number % 2 == 0) {
        if(data == NULL)    return;

        for(int y = 1; y < (CANVAS_HEIGHT/2); y+=1) {
            memcpy(uyvybuffer + ( ( (y * 2) - 1) * CANVAS_WIDTH * 2), data + (y * IMAGE_WIDTH * 2), IMAGE_WIDTH * 2);
        }

        write_gst_data(uyvybuffer, CANVAS_WIDTH*CANVAS_HEIGHT * 2);
    } else {
        if(data == NULL)    return;

        for(int y = 1; y < (CANVAS_HEIGHT/2); y+=1) {
            memcpy(uyvybuffer +           ( y* 2 * CANVAS_WIDTH * 2), data + (y * IMAGE_WIDTH * 2), IMAGE_WIDTH * 2);
        }
    }
}

#endif
