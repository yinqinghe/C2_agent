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
//// 模拟执行命令的函数
//std::string executeCommand(const std::string& cmd) {
//    // 在这里执行命令
//    // 注意：直接执行来自网络的命令可能非常危险，这里仅为示例
//    std::string result = "执行结果: " + cmd;
//    return result;
//}
//
//// 线程负责监听和接收命令
//int listenerThread() {
//    WSADATA wsaData;
//    SOCKET sock = INVALID_SOCKET;
//    struct addrinfo* result = NULL, * ptr = NULL, hints;
//    const char* sendbuf = "Hello from agent";
//    char recvbuf[512];
//    int iResult;
//    int recvbuflen = 512;
//
//    // 初始化Winsock
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
//    // 解析服务器地址和端口
//    iResult = getaddrinfo("192.168.52.132", PORT, &hints, &result);
//    if (iResult != 0) {
//        std::cout << "getaddrinfo failed with error: " << iResult << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // 尝试连接到服务器地址
//    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//        // 创建套接字
//        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
//        if (sock == INVALID_SOCKET) {
//            std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
//            WSACleanup();
//            return 1;
//        }
//
//        // 连接到服务器
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
//    // 发送数据
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
//    // 接收数据
//    iResult = recv(sock, recvbuf, recvbuflen, 0);
//    if (iResult > 0)
//        std::cout << "Bytes received: " << iResult << "nMessage: " << recvbuf << std::endl;
//    else if (iResult == 0)
//        std::cout << "Connection closed" << std::endl;
//    else
//        std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
//
//    // 关闭套接字
//    closesocket(sock);
//    WSACleanup();
//
//}
//
//// 线程负责执行命令并返回结果
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
//            // 在这里将结果发送回服务器
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
