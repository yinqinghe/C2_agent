//#include <iostream>
//#include <cstring>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <unistd.h>
//
//#define PORT 8080
//
//int main() {
//    int server_fd, new_socket;
//    struct sockaddr_in address;
//    int opt = 1;
//    int addrlen = sizeof(address);
//    char buffer[1024] = { 0 };
//    char* hello = "Hello from server";
//
//    // 创建套接字文件描述符
//    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//        perror("socket failed");
//        exit(EXIT_FAILURE);
//    }
//
//    // 绑定套接字到端口
//    address.sin_family = AF_INET;
//    address.sin_addr.s_addr = INADDR_ANY;
//    address.sin_port = htons(PORT);
//
//    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
//        perror("bind failed");
//        exit(EXIT_FAILURE);
//    }
//
//    // 监听套接字
//    if (listen(server_fd, 3) < 0) {
//        perror("listen");
//        exit(EXIT_FAILURE);
//    }
//
//    // 接受代理的连接
//    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
//        perror("accept");
//        exit(EXIT_FAILURE);
//    }
//
//    // 读取数据
//    read(new_socket, buffer, 1024);
//    std::cout << buffer << std::endl;
//
//    // 发送数据
//    send(new_socket, hello, strlen(hello), 0);
//    std::cout << "Hello message sentn";
//
//    // 关闭套接字
//    close(new_socket);
//    close(server_fd);
//
//    return 0;
//}
