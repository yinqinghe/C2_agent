//#include <opencv2/opencv.hpp>
//#include <iostream>
//
//int main() {
//    // ����VideoCapture����
//    cv::VideoCapture cap(0); // 0����Ĭ������ͷ�豸
//
//    // ����Ƿ�ɹ�������ͷ
//    if (!cap.isOpened()) {
//        std::cerr << "Error: Could not open the camera." << std::endl;
//        return -1;
//    }
//
//    // ��ȡ֡��
//    double fps = cap.get(cv::CAP_PROP_FPS);
//    if (fps == 0) {
//        fps = 30; // Ĭ��֡��Ϊ30
//    }
//
//    // ��ȡ֡���֡��
//    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
//    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
//
//    // ����VideoWriter����
//    cv::VideoWriter writer("D:\\Code\\Visual_Studio\\Mike_C2\\Mike_C2\\x64\\Debug\\output.avi",
//        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
//        fps,
//        cv::Size(frame_width, frame_height));
//
//    // ����Ƿ�ɹ�����VideoWriter
//    if (!writer.isOpened()) {
//        std::cerr << "Error: Could not open the video file for writing." << std::endl;
//        return -1;
//    }
//
//    // ����һ����������ʾ����ͷ����
//    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
//
//    // ����30�����Ƶ
//    int frames_to_capture = static_cast<int>(5 * fps);
//    for (int i = 0; i < frames_to_capture; ++i) {
//        cv::Mat frame;
//
//        // ������ͷ����һ֡
//        cap >> frame;
//
//        // ����Ƿ�ɹ�����
//        if (frame.empty()) {
//            std::cerr << "Error: Could not grab a frame." << std::endl;
//            break;
//        }
//
//        // ��֡д����Ƶ�ļ�
//        writer.write(frame);
//
//        // �ڴ�������ʾ��֡����
//        cv::imshow("Camera", frame);
//
//        // �ȴ�һ��ʱ������ʾ֡��
//        if (cv::waitKey(1000 / fps) == 27) { // ����ESC��������ǰ�˳�
//            break;
//        }
//    }
//
//    // �ͷ�����ͷ��VideoWriter��Դ
//    cap.release();
//    writer.release();
//
//    // ���ٴ���
//    cv::destroyAllWindows();
//
//    return 0;
//}
