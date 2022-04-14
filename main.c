#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "mmalencoder.h"
#include "videocapture.h"

void *video_capture_function(void *arg)
{
    while (1)
    {
        run_capture("/dev/video0");
    }
}

void *video_mmal_encoder_function(void *arg)
{
    while (1)
    {
        mmal_encoder();
    }
}

void write_gst_data(char *buffer, size_t length)
{
    mmal_new_data(buffer, length);
}

int main(void)
{
    printf("Pipeline MJPEG Encoding started \n");

    pthread_t video_mmal_encoder_thread;
    pthread_create(&video_mmal_encoder_thread, NULL, &video_mmal_encoder_function, NULL);

    pthread_t video_capture_thread;
    pthread_create(&video_capture_thread, NULL, &video_capture_function, NULL);

    pthread_exit(0);
}
