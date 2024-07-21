#include "camera_interface.hpp"

int main() {
    CameraInterface camera;

    if (!camera.openDevice()) {
        return 1;
    }

    if (!camera.initDevice()) {
        camera.closeDevice();
        return 1;
    }

    camera.startCapturing();
    camera.mainLoop();
    camera.stopCapturing();
    camera.closeDevice();

    return 0;
}
