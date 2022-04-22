#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>

struct v4l2_mmap_info {
    int index;
    int length;
    char *addr;
};
#define NUM_BUFFERS (3)

int run_capture(char *device)
{
    int i = 0;
    int status = 0;
    int v4l2_fd = 0;
    struct v4l2_format format;
    struct v4l2_requestbuffers requestbuffers;
    struct v4l2_buffer buffer;
    struct v4l2_mmap_info mmap_info[NUM_BUFFERS];

    v4l2_fd = open("/dev/video0", O_RDWR);
    if (v4l2_fd == -1) {
        printf("%d: open error\n", __LINE__);
        return -1;
    }

    memset(&format, 0x00, sizeof(struct v4l2_format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 720;
    format.fmt.pix.height = 576;
    format.fmt.pix.field = V4L2_FIELD_NONE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
    status = ioctl(v4l2_fd, VIDIOC_S_FMT, &format);
    if (status < 0) {
        printf("%d: ioctl VIDIOC_S_FMT error\n", __LINE__);
        return -1;
    }

    memset(&format, 0x00, sizeof(struct v4l2_format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    status = ioctl(v4l2_fd, VIDIOC_G_FMT, &format);
    if (status < 0) {
        printf("%d: ioctl VIDIOC_G_FMT error\n", __LINE__);
        return -1;
    }
    printf("format width: [%d]\n", format.fmt.pix.width);
    printf("format height: [%d]\n", format.fmt.pix.height);

    memset(&requestbuffers, 0x00, sizeof(struct v4l2_requestbuffers));
    requestbuffers.count = NUM_BUFFERS;
    requestbuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestbuffers.memory = V4L2_MEMORY_MMAP;
    status = ioctl(v4l2_fd, VIDIOC_REQBUFS, &requestbuffers);
    if (status < 0) {
        printf("%d: ioctl VIDIOC_REQBUFS error\n", __LINE__);
        return -1;
    }

    for (i = 0; i < requestbuffers.count; i++) {
        buffer.index = i;
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        status = ioctl(v4l2_fd, VIDIOC_QUERYBUF, &buffer);
        if (status < 0) {
            printf("%d: ioctl VIDIOC_QUERYBUF error\n", __LINE__);
            return -1;
        }

        mmap_info[i].index = buffer.index;
        mmap_info[i].length = buffer.length;
        mmap_info[i].addr = (char *)mmap(NULL,
                buffer.length,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                v4l2_fd,
                buffer.m.offset);
        if (mmap_info[i].addr == MAP_FAILED) {
            printf("%d: mmap error\n", __LINE__);
            return -1;
        }
    }

    for (i = 0; i < requestbuffers.count; i ++) {
        buffer.index = i;
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        status = ioctl(v4l2_fd, VIDIOC_QBUF, &buffer);
        if (status < 0) {
            printf("%d: ioctl VIDIOC_QBUF error\n", __LINE__);
            return -1;
        }
    }

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    status = ioctl(v4l2_fd, VIDIOC_STREAMON, &type);
    if (status < 0) {
        printf("%d: ioctl VIDIOC_STREAMOFF error\n", __LINE__);
        return -1;
    }

    FILE *fp = NULL;
    fp = fopen("frame.yuv", "wb");
    if (fp == NULL) {
        printf("%d: fopen error\n", __LINE__);
        return -1;
    }

    while(1) {
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        status = ioctl(v4l2_fd, VIDIOC_DQBUF, &buffer);
        if (status < 0) {
            printf("%d: ioctl VIDIOC_DQBUF error\n", __LINE__);
            return -1;
        }

        printf("Captured.. \n");

        decode_frame(mmap_info[buffer.index].addr, 2);

        /*
        status = fwrite(mmap_info[buffer.index].addr, mmap_info[buffer.index].length, 1, fp);
        if (status == 0) {
            printf("%d: fwrite error\n", __LINE__);
            return -1;
        }
        */

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.m.offset = (unsigned long)mmap_info[buffer.index].addr;
        status = ioctl(v4l2_fd, VIDIOC_QBUF, &buffer);
        if (status < 0) {
            printf("%d: ioctl VIDIOC_DQBUF error\n", __LINE__);
            return -1;
        }
    }

    fflush(fp);
    fclose(fp);

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    status = ioctl(v4l2_fd, VIDIOC_STREAMOFF, &type);
    if (status < 0) {
        printf("%d: ioctl VIDIOC_STREAMOFF error\n", __LINE__);
        return -1;
    }
    close (v4l2_fd);

    return status;
}
