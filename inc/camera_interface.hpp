#ifndef CAMERA_INTERFACE_HPP
#define CAMERA_INTERFACE_HPP

#include <opencv2/core.hpp>

class CameraInterface {
public:
    CameraInterface();
    ~CameraInterface();

    bool openDevice();
    void closeDevice();
    bool initDevice();
    void startCapturing();
    void stopCapturing();
    void mainLoop();

private:
    bool initMmap();
    void uninitDevice();
    void errnoExit(const char *s);

    void processImage(void *data, size_t length);

private:
    int fd;
    struct buffer *buffers;
    unsigned int nBuffers;

    static const char *cameraDevice;
    static const int captureWidth = 640;
    static const int captureHeight = 480;
    static const int numBuffers = 2;
};

#endif // CAMERA_INTERFACE_HPP
