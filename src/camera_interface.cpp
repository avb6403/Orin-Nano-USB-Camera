#include "camera_interface.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>

const char* CameraInterface::cameraDevice = "/dev/video0";

struct buffer {
    void *start;
    size_t length;
};

CameraInterface::CameraInterface() : fd(-1), buffers(nullptr), nBuffers(0) {}

CameraInterface::~CameraInterface() {
    if (fd != -1) {
        closeDevice();
    }
}

bool CameraInterface::openDevice() {
    struct v4l2_capability cap;
    if ((fd = open(cameraDevice, O_RDWR | O_NONBLOCK, 0)) == -1) {
        fprintf(stderr, "Cannot open %s: %d, %s\n", cameraDevice, errno, strerror(errno));
        return false;
    }
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is not a V4L2 device\n", cameraDevice);
            return false;
        } else {
            errnoExit("VIDIOC_QUERYCAP");
            return false;
        }
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is not a video capture device\n", cameraDevice);
        return false;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "%s does not support streaming I/O\n", cameraDevice);
        return false;
    }
    return true;
}

void CameraInterface::closeDevice() {
    if (fd != -1) {
        if (close(fd) == -1)
            errnoExit("close");
        fd = -1;
    }
}

bool CameraInterface::initDevice() {
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = captureWidth;
    fmt.fmt.pix.height = captureHeight;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
        errnoExit("VIDIOC_S_FMT");
    return initMmap();
}

bool CameraInterface::initMmap() {
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    memset(&req, 0, sizeof(req));
    req.count = numBuffers;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support memory mapping\n", cameraDevice);
            return false;
        } else {
            errnoExit("VIDIOC_REQBUFS");
            return false;
        }
    }
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", cameraDevice);
        return false;
    }
    buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));
    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        return false;
    }
    for (nBuffers = 0; nBuffers < req.count; ++nBuffers) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = nBuffers;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
            errnoExit("VIDIOC_QUERYBUF");
        buffers[nBuffers].length = buf.length;
        buffers[nBuffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (MAP_FAILED == buffers[nBuffers].start)
            errnoExit("mmap");
    }
    return true;
}

void CameraInterface::startCapturing() {
    unsigned int i;
    enum v4l2_buf_type type;
    for (i = 0; i < nBuffers; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
            errnoExit("VIDIOC_QBUF");
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
        errnoExit("VIDIOC_STREAMON");
}

void CameraInterface::mainLoop() {
    while (1) {
        fd_set fds;
        struct timeval tv;
        int r;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) {
            if (EINTR == errno)
                continue;
            errnoExit("select");
        }
        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
            errnoExit("VIDIOC_DQBUF");
        processImage(buffers[buf.index].start, buf.bytesused);
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
            errnoExit("VIDIOC_QBUF");
    }
}

void CameraInterface::stopCapturing() {
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
        errnoExit("VIDIOC_STREAMOFF");
}

void CameraInterface::uninitDevice() {
    unsigned int i;
    for (i = 0; i < nBuffers; ++i)
        if (munmap(buffers[i].start, buffers[i].length) == -1)
            errnoExit("munmap");
    free(buffers);
}

void CameraInterface::errnoExit(const char *s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

void CameraInterface::processImage(const void *p, int size) {
    cv::Mat frame(cv::Size(captureWidth, captureHeight), CV_8UC2, (void*)p);
    cv::Mat converted(cv::Size(captureWidth, captureHeight), CV_8UC3);
    cv::cvtColor(frame, converted, cv::COLOR_YUV2BGR_YUYV);
    cv::imshow("Camera", converted);
    cv::waitKey(1);
}
