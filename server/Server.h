#ifndef SERVER
#define SERVER

#include <string>
#include <unordered_map>
#include "Connection.h"

class Server{
public:
    Server(const std::string& fileName); // 服务器初始化
    void start(); // 启动服务器
    ~Server();
private:
    void parseIni(const std::string& fileName); // 解析ini配置文件，并将解析结果写到config中
    bool initSocket(); // 初始化套接字
    void setNonBlock(int fd); // 将文件描述符设置为非阻塞
    void disconnect(Connection* conn); // 断开和某个客户端的连接
    void readEvent(Connection* conn); // conn的可读事件就绪，调用该函数进行处理
    void process(Connection* conn); // 处理读出的数据
    void writeEvent(Connection* conn); // conn的可写事件就绪，调用该函数进行处理
    std::unordered_map<std::string,std::string> config; // 服务器配置
    std::unordered_map<int,Connection*> connections; // 文件描述符到Connection的映射
    bool isSuccess=true; // 服务器初始化是否成功
    int listenFd; // 用于监听的套接字的文件描述符
    uint32_t listenEvent; // 监听套接字的模式（这里使用LT水平触发模式）
    uint32_t connEvent; // 连接套接字的模式（这里使用ET边沿触发模式）
};

#endif