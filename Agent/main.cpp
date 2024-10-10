//#include <iostream>
//#include <thread>
//#include <string>
//#include <mutex>
//#include <condition_variable>
//#include <queue>
//#include <iostream>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//#define PORT "8080"
//
//std::mutex mtx;
//std::condition_variable cv;
//std::queue<std::string> commands;
//bool finished = false;
//
//// ģ��ִ������ĺ���
//std::string executeCommand(const std::string& cmd) {
//    // ������ִ������
//    // ע�⣺ֱ��ִ�����������������ܷǳ�Σ�գ������Ϊʾ��
//    std::string result = "ִ�н��: " + cmd;
//    return result;
//}
//
//// �̸߳�������ͽ�������
//int listenerThread() {
//    WSADATA wsaData;
//    SOCKET sock = INVALID_SOCKET;
//    struct addrinfo* result = NULL, * ptr = NULL, hints;
//    const char* sendbuf = "Hello from agent";
//    char recvbuf[512];
//    int iResult;
//    int recvbuflen = 512;
//
//    // ��ʼ��Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        std::cout << "WSAStartup failed with error: " << iResult << std::endl;
//        return 1;
//    }
//
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    // ������������ַ�Ͷ˿�
//    iResult = getaddrinfo("192.168.52.132", PORT, &hints, &result);
//    if (iResult != 0) {
//        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // �������ӵ���������ַ
//    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//        // �����׽���
//        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
//        if (sock == INVALID_SOCKET) {
//            std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
//            WSACleanup();
//            return 1;
//        }
//
//        // ���ӵ�������
//        iResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
//        if (iResult == SOCKET_ERROR) {
//            closesocket(sock);
//            sock = INVALID_SOCKET;
//            continue;
//        }
//        break;
//    }
//
//    freeaddrinfo(result);
//
//    if (sock == INVALID_SOCKET) {
//        std::cout << "Unable to connect to server!" << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // ��������
//    iResult = send(sock, sendbuf, (int)strlen(sendbuf), 0);
//    if (iResult == SOCKET_ERROR) {
//        std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
//        closesocket(sock);
//        WSACleanup();
//        return 1;
//    }
//
//    std::cout << "Bytes Sent: " << iResult << std::endl;
//
//    // ��������
//    iResult = recv(sock, recvbuf, recvbuflen, 0);
//    if (iResult > 0)
//        std::cout << "Bytes received: " << iResult << "nMessage: " << recvbuf << std::endl;
//    else if (iResult == 0)
//        std::cout << "Connection closed" << std::endl;
//    else
//        std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
//
//    // �ر��׽���
//    closesocket(sock);
//    WSACleanup();
//
//}
//
//// �̸߳���ִ��������ؽ��
//void executorThread() {
//    while (!finished || !commands.empty()) {
//        std::unique_lock<std::mutex> lock(mtx);
//        cv.wait(lock, [] {return !commands.empty() || finished; });
//
//        while (!commands.empty()) {
//            std::string cmd = commands.front();
//            commands.pop();
//            lock.unlock();
//
//            std::string result = executeCommand(cmd);
//            // �����ｫ������ͻط�����
//            std::cout << result << std::endl;
//
//            lock.lock();
//        }
//    }
//}
//
//int main() {
//    std::thread listener(listenerThread);
//    std::thread executor(executorThread);
//
//    listener.join();
//    executor.join();
//
//    return 0;
//}
