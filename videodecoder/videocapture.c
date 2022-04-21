#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>

#include "videodecoder.h"

typedef struct
{
    void *start;
    size_t size;
    struct timespec time;
    int index;
    int id;
} buffer_t;

// Internal camera buffers
#define CAMERA_NUM_BUF 5

// mmap buffers
buffer_t frame_buffers[CAMERA_NUM_BUF];

static int xioctl(int fd, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}
 
int print_caps(int fd)
{
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = IMAGE_WIDTH;
    fmt.fmt.pix.height = IMAGE_HEIGHT;

#ifdef __arm__
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
#else
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
#endif
    
    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        perror("Setting Pixel Format");
        return 1;
    }

    printf("Selected Camera Mode:\n"
            "  Width: %d\n"
            "  Height: %d\n"
            "  Field: %d\n",
            fmt.fmt.pix.width,
            fmt.fmt.pix.height,
            fmt.fmt.pix.field);
    return 0;
}

int init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = CAMERA_NUM_BUF;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
 
    for (int i = 0; i < CAMERA_NUM_BUF; i++)
    {
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            perror("Querying Buffer");
            return 1;
        }

        frame_buffers[i].size = buf.length;
        frame_buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        printf("Length: %d \n", frame_buffers[i].size);
        printf("Image Length: %d \n", buf.bytesused);
    }

    return 0;
}

int deinit_mmap(int fd)
{
    unsigned int i;
    
    for (i = 0; i < CAMERA_NUM_BUF; i++)
    {
        if (-1 == munmap(frame_buffers[i].start, frame_buffers[i].size))
        {
            perror("munmap");
            return 1;
        }
    }

    return 0;
}

int queue_buffer(int fd, int id)
{
        struct v4l2_buffer buf = {0};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = id;
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        {
            perror("Queue Buffer");
            return 1;
        }
    
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
        {
            perror("Start Capture");
            return 1;
        }

        return 0;
}

void start_capture(int fd)
{

    for (int i = 0; i < CAMERA_NUM_BUF; i++)
    {
        // Enqueue buffer
        //
        queue_buffer(fd, i);
    }
}

int stop_capture(int fd)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
    {
            perror("VIDIOC_STREAMOFF");
            return 1;
    }

    return 0;
}

long long current_timestamp()
{
    struct timeval te; 
    gettimeofday(&te, NULL);                                         // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
    return milliseconds;
}

bool waiting_for_top_frame = true;

int capture_image(int fd)
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 1;
    int r = select(fd + 1, &fds, NULL, NULL, &tv);
    if (-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }

    if (r == 0)
    {
        printf("Timeout occured! \n");
        return -1;
    }

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }

    char *outbuf = NULL;

    outbuf = decode_frame(frame_buffers[buf.index].start, 2);

    queue_buffer(fd, buf.index);

    return outbuf;
}

int run_capture(char *device)
{
    int fd;

#ifdef __arm__
    system("v4l2-ctl --set-standard=5");
#endif

    decode_init();

    fd = open(device, O_RDWR);
    if (fd == -1)
    {
        perror("Opening video device");
        return 1;
    }

    if (print_caps(fd))
    {
        return 1;
    }
    
    if (init_mmap(fd))
    {
        return 1;
    }

    start_capture(fd);

    return fd;
}
