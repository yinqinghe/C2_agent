//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//
//int main() {
//    std::string filename = "D:\\Code\\Visual_Studio\\Mike_C2\\Mike_C2\\x64\\Debug\\output.txt"; // �滻Ϊ����ļ�����·��
//    std::vector<char> buffer = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
//
//    // ���ļ�
//    std::ofstream file(filename, std::ios::binary);
//    if (!file) {
//        std::cerr << "Could not open file for writing: " << filename << std::endl;
//        return 1;
//    }
//
//    // д�����ݵ��ļ�
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
//    std::cout << "�ļ�������ϣ��� �ļ���С: " << buffer.size() << " bytes" << std::endl;
//    return 0;
//}
