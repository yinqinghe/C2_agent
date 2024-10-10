//#include <iostream>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//
//#pragma comment(lib, "Ws2_32.lib")
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "8080"
//
//int main() {
//    WSADATA wsaData;
//    SOCKET ConnectSocket = INVALID_SOCKET;
//    struct addrinfo* result = NULL, * ptr = NULL, hints;
//    char recvbuf[DEFAULT_BUFLEN];
//    int iResult;
//    int recvbuflen = DEFAULT_BUFLEN;
//
//    // 初始化Winsock
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
//    // 解析服务器地址和端口
//    iResult = getaddrinfo("192.168.52.132", DEFAULT_PORT, &hints, &result);
//    if (iResult != 0) {
//        std::cout << "getaddrinfo failed: " << iResult << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // 尝试连接到服务器
//    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//
//        // 创建套接字
//        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
//        if (ConnectSocket == INVALID_SOCKET) {
//            std::cout << "socket failed: " << WSAGetLastError() << std::endl;
//            WSACleanup();
//            return 1;
//        }
//
//        // 连接到服务器
//        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//        if (iResult == SOCKET_ERROR) {
//            closesocket(ConnectSocket);
//            ConnectSocket = INVALID_SOCKET;
//            continue;
//        }
//        break;
//    }
//
//    freeaddrinfo(result);
//
//    if (ConnectSocket == INVALID_SOCKET) {
//        std::cout << "Unable to connect to server!" << std::endl;
//        WSACleanup();
//        return 1;
//    }
//
//    // 接收数据循环
//    do {
//        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//        if (iResult > 0) {
//            std::cout << "Bytes received: " << iResult << std::endl;
//            std::cout << "Message: " << std::string(recvbuf, 0, iResult) << std::endl;
//        }
//        else if (iResult == 0)
//            std::cout << "Connection closed" << std::endl;
//        else
//            std::cout << "recv failed: " << WSAGetLastError() << std::endl;
//    } while (iResult > 0);
//
//    // 关闭套接字
//    closesocket(ConnectSocket);
//    WSACleanup();
//
//    return 0;
//}
