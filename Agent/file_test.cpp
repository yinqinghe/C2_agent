//#include <iostream>
//#include <filesystem>
//#include <string>
//
//namespace fs = std::filesystem;
//
//// ���� directory_path �ǿͻ��������Ŀ¼·��
//void ListDirectory(const std::string& directory_path) {
//    try {
//        if (fs::exists(directory_path) && fs::is_directory(directory_path)) {
//            for (const auto& entry : fs::directory_iterator(directory_path)) {
//                const auto filenameStr = entry.path().filename().string();
//                if (entry.is_directory()) {
//                    std::cout << "[DIR]  " << filenameStr << std::endl;
//                }
//                else if (entry.is_regular_file()) {
//                    std::cout << "[FILE] " << filenameStr << std::endl;
//                }
//                else {
//                    std::cout << "[OTHER] " << filenameStr << std::endl;
//                }
//            }
//        }
//        else {
//            std::cout << "Path does not exist or is not a directory." << std::endl;
//        }
//    }
//    catch (const fs::filesystem_error& err) {
//        std::cerr << "Filesystem error: " << err.what() << std::endl;
//    }
//    catch (const std::exception& e) {
//        std::cerr << "Error: " << e.what() << std::endl;
//    }
//}
//
//int main() {
//    // ����ListDirectory����������Ҫ�������ϵ���ķ�����ͨ���߼���ȥ
//    std::string directory_path;
//    std::cout << "Enter directory path to list: ";
//    std::cin >> directory_path;
//    ListDirectory(directory_path);
//
//    return 0;
//}
