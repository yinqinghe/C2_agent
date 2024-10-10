#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <queue>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <functional>
#include <string>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"
#define RECONNECT_DELAY 5000 // 重连延迟时间（毫秒）


class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    // 添加任务到线程池
    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(task);
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

void handleListen(SOCKET agentSock) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    std::cout << "Waiting from Server: " << std::endl;
    while (true) {
        std::memset(buffer, 0, bufferSize);
        int bytesReceived = recv(agentSock, buffer, bufferSize, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }
        std::cout << "Bytes received: " << bytesReceived << std::endl;
        std::cout << "Messagess(Client): " << std::string(buffer, 0, bytesReceived) << std::endl;

        // 处理agent代理发来的消息...
    }

    closesocket(agentSock);
}

void handleClient(SOCKET clientSock) {     //给Server服务器发送信息
    const int bufferSize = 1024;
    char buffer[bufferSize];

    struct sockaddr_in sa;
    // 假设sa已被赋值
    char ip[INET_ADDRSTRLEN]; // INET_ADDRSTRLEN 是 IPv4 地址的字符串长度
    inet_ntop(AF_INET, &(sa.sin_addr), ip, INET_ADDRSTRLEN);

    while (true) {
        std::cout << "Enter message to server(" << ip << "): ";
        std::string msg;
        std::getline(std::cin, msg);

        if (msg == "exit") {
            closesocket(clientSock);
            WSACleanup();
            break;
        }

        int sentBytes = -1;
        int retries = 5; // 设置重试次数
        while (retries > 0) {
            sentBytes = send(clientSock, msg.c_str(), msg.length(), 0);
            if (sentBytes == SOCKET_ERROR) {
                int errorCode = WSAGetLastError();
                std::cerr << "Failed to send message. Error: " << errorCode << std::endl;

                if (errorCode == WSAEWOULDBLOCK) {
                    // 非阻塞socket资源不足，可稍后重试
                    std::cerr << "Resource temporarily unavailable. Retrying..." << std::endl;
                    Sleep(1000); // 等待1秒
                }
                else {
                    // 对于其它错误，可能需要处理或记录错误
                    retries--; // 仅当需要重试时减少重试计数
                }
            }
            else {
                std::cout << "Message sent successfully" << std::endl;
                break; // 跳出循环
            }
        }
        if (sentBytes == -1) {
            std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
            break;
            // 处理连接关闭的情况
        }

    }
    closesocket(clientSock);
    WSACleanup();
    std::cout << "Connection closed for socket: " << clientSock << std::endl;
}

bool sendMessage(int sock, const std::string& message) {
    int sentBytes = send(sock, message.c_str(), message.length(), 0);
    if (sentBytes < 0) {
        std::cerr << "Failed to send message." << std::endl;
        return false;
    }
    return true;
}



int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    ThreadPool pool(4);

    // 初始化Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    bool stayConnected = true; // 控制重连接逻辑
    //while (stayConnected) { // 主循环 - 持续尝试连接和接收数据
        // 解析服务器地址和端口
        iResult = getaddrinfo("192.168.52.132", DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            std::cout << "getaddrinfo failed: " << iResult << std::endl;
            WSACleanup();
            return 1;
        }

        // 尝试连接到服务器
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            // 创建套接字
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (ConnectSocket == INVALID_SOCKET) {
                std::cout << "socket failed: " << WSAGetLastError() << std::endl;
                WSACleanup();
                return 1;
            }

            // 连接到服务器
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }
            break;
        }

        freeaddrinfo(result);

        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Unable to connect to server, will try again in " << RECONNECT_DELAY << "ms" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
            return -1;
        }
        std::cout << "Connection successfully,Sending Messages" << std::endl;

        // 连接建立后，立即发送"AGENT"标识
        if (!sendMessage(ConnectSocket, "CLIENT")) {
            closesocket(ConnectSocket);
            stayConnected = false; // 假设我们不想无限重试，则在这里退出循环
            return -1;
        }

        // 将新连接的处理任务添加到线程池
        pool.enqueue([ConnectSocket] {
            handleClient(ConnectSocket);
            });
        pool.enqueue([ConnectSocket] {
            handleListen(ConnectSocket);
            });

//// 等待一段时间后重试连接
//        std::cout << "Will try to reconnect in " << RECONNECT_DELAY << "ms" << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
    //}


    //WSACleanup();

    return 0;
}
