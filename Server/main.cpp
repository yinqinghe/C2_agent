//#include <iostream>
//#include <string>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <unistd.h>
//
//// 假设代理是一个简单的函数，执行命令并返回结果
//std::string executeCommand(const std::string& cmd) {
//    // 在这里执行命令，例如使用 system() 函数
//    // 但出于安全考虑，不推荐在实际应用中使用 system()
//    // 应该使用更安全的替代方案
//    return "执行结果";
//}
//
//int main() {
//    int server_fd, new_socket;
//    struct sockaddr_in address;
//    int opt = 1;
//    int addrlen = sizeof(address);
//    char buffer[1024] = { 0 };
//    std::string command;
//
//    // 创建套接字
//    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//        perror("socket failed");
//        exit(EXIT_FAILURE);
//    }
//
//    // 绑定套接字到端口
//    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//        perror("setsockopt");
//        exit(EXIT_FAILURE);
//    }
//    address.sin_family = AF_INET;
//    address.sin_addr.s_addr = INADDR_ANY;
//    address.sin_port = htons(9366);
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
//    // 接受客户端连接
//    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
//        perror("accept");
//        exit(EXIT_FAILURE);
//    }
//
//    // 读取命令
//    read(new_socket, buffer, 1024);
//    command = buffer;
//
//    // 执行命令并获取结果
//    std::string result = executeCommand(command);
//
//    // 发送结果回客户端
//    send(new_socket, result.c_str(), result.size(), 0);
//
//    // 关闭套接字
//    close(new_socket);
//    close(server_fd);
//
//    return 0;
//}
