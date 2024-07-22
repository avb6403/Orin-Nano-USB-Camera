#include "gui.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>



void drawGUI(cv::Mat& frame) {
    // Add GUI elements here
    // Example: Draw a rectangle
    cv::rectangle(frame, cv::Point(50, 50), cv::Point(200, 200), cv::Scalar(0, 255, 0), 2);

    // Example: Draw text
    cv::putText(frame, "Face Detection App", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

    // Example: Change color scheme
    // cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY); // Convert to grayscale
}

void faceDetect(cv::VideoCapture& cap) {
    cv::CascadeClassifier faceCascade;
    faceCascade.load("/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml");

    cv::CascadeClassifier eyeCascade;
    eyeCascade.load("/usr/share/opencv4/haarcascades/haarcascade_eye.xml");

    cv::Mat frame;
    cv::namedWindow("Face Detect", cv::WINDOW_AUTOSIZE);

    while (true) {
        cap >> frame;

        if (frame.empty()) {
            break;
        }

        cv::Mat frameGray;
        cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(frameGray, faces, 1.3, 5);

        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(255, 0, 0), 2);

            cv::Mat faceROI = frameGray(face);
            std::vector<cv::Rect> eyes;
            eyeCascade.detectMultiScale(faceROI, eyes);

            for (const auto& eye : eyes) {
                cv::Point eyeCenter(face.x + eye.x + eye.width / 2, face.y + eye.y + eye.height / 2);
                int radius = cvRound((eye.width + eye.height) * 0.25);
                cv::circle(frame, eyeCenter, radius, cv::Scalar(0, 255, 0), 2);
            }
        }

        drawGUI(frame);

        cv::imshow("Face Detect", frame);
        int key = cv::waitKey(30);

        if (key == 27 || key == 'q') {
            break;
        }
    }
}

