#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <queue>
#include <arpa/inet.h> 
#include <map>
using namespace std;
#include <vector>
#define PORT 9363
#include <csignal>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <memory>
std::mutex connMutex;
std::map<int, int> clientAgentMap; // 映射客户端和代理的套接字
std::vector<int> agentSockets; // 存储所有代理的套接字

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


void handleListen(int agentSock) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    //std::cout << "Waiting from agent: " << std::endl;
    while (true) {
        std::memset(buffer, 0, bufferSize);
        int bytesReceived = recv(agentSock, buffer, bufferSize, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }
        std::cout << "Bytes received: " << bytesReceived << std::endl;
        std::cout << "Messagess: " << std::string(buffer, 0, bytesReceived) << std::endl;

        // 处理agent代理发来的消息...
    }

    close(agentSock);
}

void handleClient(int clientSock) {     //给agent代理发送信息
    const int bufferSize = 1024;
    char buffer[bufferSize];
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    char* clientIP = inet_ntoa(clientAddress.sin_addr);
    unsigned short clientPort = ntohs(clientAddress.sin_port);

    //std::cout << "Agent connected with IP: " << clientIP << " and port: " << clientPort << "\n" << clientAddress.sin_family << std::endl;
    while (true) {
        std::cout << "Enter message to agent(" << clientIP << "): ";
        std::string msg;
        std::getline(std::cin, msg);

        if (msg == "exit") {
            break;
        }
        ssize_t sentBytes = -1;
        int retries = 5; // 设置重试次数
        while (retries > 0) {
            sentBytes = send(clientSock, msg.c_str(), msg.length(), 0);
            if (sentBytes != -1) break; // 发送成功，退出循环
            retries--;
            std::cerr << "Failed to send message, retrying..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待一段时间后重试
        }
        if (sentBytes == -1) {
            std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
            break;
            // 处理连接关闭的情况
        }

    }

    close(clientSock);
    std::cout << "Connection closed for socket: " << clientSock << std::endl;
}

void AgentToClient(int sock) {
    ssize_t bytesReceived;
    char buffer[1024 * 1024*4];
    while (true) {
        
        bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }
        // 将代理的消息转发给对应的客户端
        std::string message(buffer);
        if (bytesReceived > 1024 * 16) {
            cout << "bytesReceived:  "  << " | " << bytesReceived << endl;
        }
        else {
            cout << "Agent to Client Message:  " << message << " | " << bytesReceived << endl;
        }


        std::lock_guard<std::mutex> lock(connMutex);
        int clientSock = clientAgentMap[sock]; // 找到对应的客户端套接字
        int sentBytes=send(clientSock, buffer, bytesReceived, 0); // 转发消息
        if (sentBytes == -1) {
            std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
            break;
            // 处理连接关闭的情况
        }
    }
}

void ClientToAgent(int sock) {
    char buffer[1024*1024*4];
    ssize_t bytesReceived;
    // 客户端连接处理逻辑
    while (true) {
        bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }
        // 将客户端的消息转发给代理
        std::lock_guard<std::mutex> lock(connMutex);

        cout << "agentSockets.empty:  " << agentSockets.empty() << endl;
        if (!agentSockets.empty()) {
            std::string message(buffer);
            cout << "bytesReceived:  " << " | " << bytesReceived << endl;
      /*      if (bytesReceived > 1024 * 4) {
                cout << "bytesReceived:  " << " | " << bytesReceived << endl;
            }
            else {
                cout << "Client to Client Message:  " << message << " | " << bytesReceived << endl;
            }*/
            int agentSock = agentSockets.back(); // 选择一个代理
            clientAgentMap[agentSock] = sock; // 建立映射关系
            int sentBytes=send(agentSock, buffer, bytesReceived, 0); // 转发消息
            if (sentBytes == -1) {
                std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
                break;
                // 处理连接关闭的情况
            }
        }
    }
    close(sock);
    std::cout << "Connection closed for socket: " << sock << std::endl;
}

// 接收连接方发送的第一个消息，并根据消息内容确定连接类型
bool determineIfAgent(int sock) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // 确保字符串以null结尾
        std::string message(buffer);
        std::cout << "First Message: " <<message<< std::endl;

        if (message == "AGENT") {
            return true; // 是代理
        }
        else if (message == "CLIENT") {

            return false; // 是客户端
        }
    }
    return false; // 默认为客户端，或者处理错误/无效的情况
}

int serverFd; // 全局变量来存储服务器套接字描述符
volatile sig_atomic_t keepRunning = 1;

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.n";
    keepRunning = 0;
    // 关闭服务器套接字
    if (serverFd) {
        close(serverFd);
        std::cout << "Server socket closed.n";
    }

    // 退出程序
    exit(signum);
}

void setupSignalHandler() {
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int serverFd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    std::vector<std::thread> threads;
    ThreadPool pool(4);

    // 设置信号处理函数
    setupSignalHandler();

    // 创建套接字
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 绑定套接字到端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听套接字
    if (listen(serverFd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (keepRunning) {
        std::cout << "Waiting for agent connections ..." << std::endl;
        // 接受代理连接
        int newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (newSocket < 0) {
            perror("accept");
            continue; // 如果接受失败，继续监听下一个连接
        }
        bool isAgent = determineIfAgent(newSocket);
        std::cout << "isAgent: " <<isAgent << std::endl;
        if (isAgent) {
            {
                std::lock_guard<std::mutex> lock(connMutex);
                agentSockets.push_back(newSocket); // 保存代理套接字
            }
            pool.enqueue([newSocket] {  AgentToClient(newSocket); });

        }else {
            pool.enqueue([newSocket] {  ClientToAgent(newSocket); });
        }
    }
    close(serverFd);

    return 0;
}
