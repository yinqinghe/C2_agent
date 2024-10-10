//#include <opencv2/opencv.hpp>
//#include <iostream>
//
//int main() {
//    // 创建VideoCapture对象
//    cv::VideoCapture cap(0); // 0代表默认摄像头设备
//
//    // 检查是否成功打开摄像头
//    if (!cap.isOpened()) {
//        std::cerr << "Error: Could not open the camera." << std::endl;
//        return -1;
//    }
//
//    // 获取帧率
//    double fps = cap.get(cv::CAP_PROP_FPS);
//    if (fps == 0) {
//        fps = 30; // 默认帧率为30
//    }
//
//    // 获取帧宽和帧高
//    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
//    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
//
//    // 创建VideoWriter对象
//    cv::VideoWriter writer("D:\\Code\\Visual_Studio\\Mike_C2\\Mike_C2\\x64\\Debug\\output.avi",
//        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
//        fps,
//        cv::Size(frame_width, frame_height));
//
//    // 检查是否成功创建VideoWriter
//    if (!writer.isOpened()) {
//        std::cerr << "Error: Could not open the video file for writing." << std::endl;
//        return -1;
//    }
//
//    // 创建一个窗口来显示摄像头画面
//    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
//
//    // 捕获30秒的视频
//    int frames_to_capture = static_cast<int>(5 * fps);
//    for (int i = 0; i < frames_to_capture; ++i) {
//        cv::Mat frame;
//
//        // 从摄像头捕获一帧
//        cap >> frame;
//
//        // 检查是否成功捕获
//        if (frame.empty()) {
//            std::cerr << "Error: Could not grab a frame." << std::endl;
//            break;
//        }
//
//        // 将帧写入视频文件
//        writer.write(frame);
//
//        // 在窗口中显示这帧画面
//        cv::imshow("Camera", frame);
//
//        // 等待一段时间以显示帧率
//        if (cv::waitKey(1000 / fps) == 27) { // 按下ESC键可以提前退出
//            break;
//        }
//    }
//
//    // 释放摄像头和VideoWriter资源
//    cap.release();
//    writer.release();
//
//    // 销毁窗口
//    cv::destroyAllWindows();
//
//    return 0;
//}
