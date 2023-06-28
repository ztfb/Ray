#ifndef SERVER
#define SERVER

#include <string>
#include <unordered_map>

class Server{
public:
    Server(const std::string& fileName); // 服务器初始化
    void start(); // 启动服务器
    ~Server();
private:
    void parseIni(const std::string& fileName); // 解析ini配置文件，并将解析结果写到config中
    bool initSocket(); // 初始化套接字
    void setNonBlock(int fd); // 将文件描述符设置为非阻塞
    std::unordered_map<std::string,std::string> config; // 服务器配置
    bool isSuccess=true; // 服务器初始化是否成功
    int listenFd; // 用于监听的套接字的文件描述符
};

#endif