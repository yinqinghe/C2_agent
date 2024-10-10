//#include <iostream>
//#include <cstring>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <unistd.h>
//#include <thread>
//#include <queue>
//#include <arpa/inet.h> 
//using namespace std;
//#include <vector>
//#define PORT 8080
//
//#include <mutex>
//#include <condition_variable>
//#include <functional>
//#include <atomic>
//#include <memory>
//
//class ThreadPool {
//public:
//    ThreadPool(size_t threads) : stop(false) {
//        for (size_t i = 0; i < threads; ++i) {
//            workers.emplace_back([this] {
//                while (true) {
//                    std::function<void()> task;
//                    {
//                        std::unique_lock<std::mutex> lock(this->queue_mutex);
//                        this->condition.wait(lock,
//                            [this] { return this->stop || !this->tasks.empty(); });
//                        if (this->stop && this->tasks.empty())
//                            return;
//                        task = std::move(this->tasks.front());
//                        this->tasks.pop();
//                    }
//                    task();
//                }
//                });
//        }
//    }
//
//    // ��������̳߳�
//    void enqueue(std::function<void()> task) {
//        {
//            std::unique_lock<std::mutex> lock(queue_mutex);
//            if (stop)
//                throw std::runtime_error("enqueue on stopped ThreadPool");
//            tasks.emplace(task);
//        }
//        condition.notify_one();
//    }
//
//    ~ThreadPool() {
//        {
//            std::unique_lock<std::mutex> lock(queue_mutex);
//            stop = true;
//        }
//        condition.notify_all();
//        for (std::thread& worker : workers)
//            worker.join();
//    }
//
//private:
//    std::vector<std::thread> workers;
//    std::queue<std::function<void()>> tasks;
//
//    std::mutex queue_mutex;
//    std::condition_variable condition;
//    std::atomic<bool> stop;
//};
//
//
//void handleAgent(int agentSock) {
//    const int bufferSize = 1024;
//    char buffer[bufferSize];
//    std::cout << "Waiting from agent: " << std::endl;
//    while (true) {
//        std::memset(buffer, 0, bufferSize);
//        int bytesReceived = recv(agentSock, buffer, bufferSize, 0);
//        if (bytesReceived <= 0) {
//            std::cerr << "Error in recv(). Quitting" << std::endl;
//            break;
//        }
//        std::cout << "Bytes received: " << bytesReceived << std::endl;
//        std::cout << "Messagess: " << std::string(buffer, 0, bytesReceived) << std::endl;
//
//        // ����agent����������Ϣ...
//    }
//
//    close(agentSock);
//}
//
//void handleClient(int clientSock) {     //��agent��������Ϣ
//    const int bufferSize = 1024;
//    char buffer[bufferSize];
//    struct sockaddr_in clientAddress;
//    socklen_t clientAddressLen = sizeof(clientAddress);
//
//    char* clientIP = inet_ntoa(clientAddress.sin_addr);
//    unsigned short clientPort = ntohs(clientAddress.sin_port);
//
//    //std::cout << "Agent connected with IP: " << clientIP << " and port: " << clientPort << "\n" << clientAddress.sin_family << std::endl;
//    while (true) {
//        std::cout << "Enter message to agent("<<clientIP<<"): ";
//        std::string msg;
//        std::getline(std::cin, msg);
//
//        if (msg == "exit") {
//            break;
//        }
//
//        ssize_t sentBytes = -1;
//        int retries = 5; // �������Դ���
//        while (retries > 0) {
//            sentBytes = send(clientSock, msg.c_str(), msg.length(), 0);
//            if (sentBytes != -1) break; // ���ͳɹ����˳�ѭ��
//            retries--;
//            std::cerr << "Failed to send message, retrying..." << std::endl;
//            std::this_thread::sleep_for(std::chrono::seconds(1)); // �ȴ�һ��ʱ�������
//        }
//        if (sentBytes == -1) {
//            std::cerr << "Failed to send message after retries. Connection might be closed." << std::endl;
//            break;
//            // �������ӹرյ����
//        }
//
//    }
//
//    close(clientSock);
//    std::cout << "Connection closed for socket: " << clientSock << std::endl;
//}
//
//int main() {
//    int serverFd, newSocket;
//    struct sockaddr_in address;
//    int opt = 1;
//    int addrlen = sizeof(address);
//    std::vector<std::thread> threads;
//    ThreadPool pool(4);
//
//    // �����׽���
//    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//        perror("socket failed");
//        exit(EXIT_FAILURE);
//    }
//
//    // ���׽��ֵ��˿�
//    address.sin_family = AF_INET;
//    address.sin_addr.s_addr = INADDR_ANY;
//    address.sin_port = htons(PORT);
//
//    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
//        perror("bind failed");
//        exit(EXIT_FAILURE);
//    }
//
//    // �����׽���
//    if (listen(serverFd, 3) < 0) {
//        perror("listen");
//        exit(EXIT_FAILURE);
//    }
//
//
//    while (true) {
//        std::cout << "Waiting for agent connections ..." << std::endl;
//        // ���ܴ�������
//        int newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
//        if (newSocket < 0) {
//            perror("accept");
//            continue; // �������ʧ�ܣ�����������һ������
//        }
//      
//        // �������ӵĴ���������ӵ��̳߳�
//        pool.enqueue([newSocket] {
//            handleClient(newSocket);
//            });
//        pool.enqueue([newSocket] {
//            handleAgent(newSocket);
//            });
//    }
//    //while (true) {
//    //    // ���ܴ�������
//    //    int newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
//    //    if (newSocket < 0) {
//    //        perror("accept");
//    //        continue; // �������ʧ�ܣ�����������һ������
//    //    }
//
//    //    std::cout << "Agent connected" << std::endl;
//    //    // Ϊÿ���������Ӵ���һ���µ��߳�
//    //    threads.emplace_back(std::thread(handleClient, newSocket));
//    //    threads.emplace_back(std::thread(handleAgent, newSocket));
//
//    //}
//
//    //// �ȴ������߳����
//    //for (auto& t : threads) {
//    //    if (t.joinable()) {
//    //        t.join();
//    //    }
//    //}
//
//
//    //// ���������̣߳�һ�����ڴ���ͻ��ˣ���һ�����ڴ������
//    //std::thread clientThread(handleClient, newSocket);
//    //std::thread agentThread(handleAgent, newSocket);
//
//    //clientThread.join();
//    //agentThread.join();
//
//    close(serverFd);
//
//    return 0;
//}
