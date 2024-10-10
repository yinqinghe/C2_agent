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
std::map<int, int> clientAgentMap; // ӳ��ͻ��˺ʹ�����׽���
std::vector<int> agentSockets; // �洢���д�����׽���

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

    // ��������̳߳�
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

        // ����agent����������Ϣ...
    }

    close(agentSock);
}

void handleClient(int clientSock) {     //��agent��������Ϣ
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
        int retries = 5; // �������Դ���
        while (retries > 0) {
            sentBytes = send(clientSock, msg.c_str(), msg.length(), 0);
            if (sentBytes != -1) break; // ���ͳɹ����˳�ѭ��
            retries--;
            std::cerr << "Failed to send message, retrying..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // �ȴ�һ��ʱ�������
        }
        if (sentBytes == -1) {
            std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
            break;
            // �������ӹرյ����
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
        // ���������Ϣת������Ӧ�Ŀͻ���
        std::string message(buffer);
        if (bytesReceived > 1024 * 16) {
            cout << "bytesReceived:  "  << " | " << bytesReceived << endl;
        }
        else {
            cout << "Agent to Client Message:  " << message << " | " << bytesReceived << endl;
        }


        std::lock_guard<std::mutex> lock(connMutex);
        int clientSock = clientAgentMap[sock]; // �ҵ���Ӧ�Ŀͻ����׽���
        int sentBytes=send(clientSock, buffer, bytesReceived, 0); // ת����Ϣ
        if (sentBytes == -1) {
            std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
            break;
            // �������ӹرյ����
        }
    }
}

void ClientToAgent(int sock) {
    char buffer[1024*1024*4];
    ssize_t bytesReceived;
    // �ͻ������Ӵ����߼�
    while (true) {
        bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }
        // ���ͻ��˵���Ϣת��������
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
            int agentSock = agentSockets.back(); // ѡ��һ������
            clientAgentMap[agentSock] = sock; // ����ӳ���ϵ
            int sentBytes=send(agentSock, buffer, bytesReceived, 0); // ת����Ϣ
            if (sentBytes == -1) {
                std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
                break;
                // �������ӹرյ����
            }
        }
    }
    close(sock);
    std::cout << "Connection closed for socket: " << sock << std::endl;
}

// �������ӷ����͵ĵ�һ����Ϣ����������Ϣ����ȷ����������
bool determineIfAgent(int sock) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // ȷ���ַ�����null��β
        std::string message(buffer);
        std::cout << "First Message: " <<message<< std::endl;

        if (message == "AGENT") {
            return true; // �Ǵ���
        }
        else if (message == "CLIENT") {

            return false; // �ǿͻ���
        }
    }
    return false; // Ĭ��Ϊ�ͻ��ˣ����ߴ������/��Ч�����
}

int serverFd; // ȫ�ֱ������洢�������׽���������
volatile sig_atomic_t keepRunning = 1;

// �źŴ�����
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.n";
    keepRunning = 0;
    // �رշ������׽���
    if (serverFd) {
        close(serverFd);
        std::cout << "Server socket closed.n";
    }

    // �˳�����
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

    // �����źŴ�����
    setupSignalHandler();

    // �����׽���
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // ���׽��ֵ��˿�
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // �����׽���
    if (listen(serverFd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (keepRunning) {
        std::cout << "Waiting for agent connections ..." << std::endl;
        // ���ܴ�������
        int newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (newSocket < 0) {
            perror("accept");
            continue; // �������ʧ�ܣ�����������һ������
        }
        bool isAgent = determineIfAgent(newSocket);
        std::cout << "isAgent: " <<isAgent << std::endl;
        if (isAgent) {
            {
                std::lock_guard<std::mutex> lock(connMutex);
                agentSockets.push_back(newSocket); // ��������׽���
            }
            pool.enqueue([newSocket] {  AgentToClient(newSocket); });

        }else {
            pool.enqueue([newSocket] {  ClientToAgent(newSocket); });
        }
    }
    close(serverFd);

    return 0;
}
