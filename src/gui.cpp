#include "gui.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

GUI::GUI() : defaultColorScheme(true) {}

GUI::~GUI() {}

void GUI::createWindow(const std::string& windowName, int width, int height) {
    this->windowName = windowName;
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);  // Create a resizable window
    cv::resizeWindow(windowName, width, height);     // Initial size
}

void GUI::showImage(const cv::Mat& image) {
    currentImage = image.clone();  // Store a copy of the image
    applyColorScheme(currentImage);  // Apply color scheme
    cv::imshow(windowName, currentImage);
}

void GUI::processEvents() {
    cv::waitKey(1);  // Allow time for GUI events
}

void GUI::changeColorScheme(bool useDefaultColorScheme) {
    defaultColorScheme = useDefaultColorScheme;
}

void GUI::resizeWindow(int width, int height) {
    cv::resizeWindow(windowName, width, height);
}

void GUI::applyColorScheme(cv::Mat& image) {
    // Example: Implementing a color inversion scheme
    if (!defaultColorScheme) {
        cv::bitwise_not(image, image);
    }
    // Add more custom color schemes as needed
}
