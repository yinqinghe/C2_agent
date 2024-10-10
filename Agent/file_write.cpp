//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//
//int main() {
//    std::string filename = "D:\\Code\\Visual_Studio\\Mike_C2\\Mike_C2\\x64\\Debug\\output.txt"; // 替换为你的文件名或路径
//    std::vector<char> buffer = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
//
//    // 打开文件
//    std::ofstream file(filename, std::ios::binary);
//    if (!file) {
//        std::cerr << "Could not open file for writing: " << filename << std::endl;
//        return 1;
//    }
//
//    // 写入数据到文件
//    file.write(buffer.data(), buffer.size());
//    if (!file) {
//        std::cerr << "Error writing to file: " << filename << std::endl;
//        return 1;
//    }
//
//    file.close();
//    if (!file) {
//        std::cerr << "Error closing file: " << filename << std::endl;
//        return 1;
//    }
//
//    std::cout << "文件传输完毕！！ 文件大小: " << buffer.size() << " bytes" << std::endl;
//    return 0;
//}
