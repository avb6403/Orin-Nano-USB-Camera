#ifndef GUI_HPP
#define GUI_HPP

#include <opencv2/core.hpp>

class GUI {
public:
    GUI();
    ~GUI();
    
    void createWindow(const std::string& windowName, int width, int height);
    void showImage(const cv::Mat& image);
    void processEvents();
    void changeColorScheme(bool useDefaultColorScheme);
    void resizeWindow(int width, int height);

private:
    cv::Mat currentImage;
    std::string windowName;
    bool defaultColorScheme;  // Flag to switch between default and custom color schemes

    // Helper functions
    void applyColorScheme(cv::Mat& image);
};

#endif // GUI_HPP
