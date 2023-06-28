#ifndef CONNECTION
#define CONNECTION

#include <sys/uio.h>
#include <arpa/inet.h>
#include <string>
#include "Buffer.h"

class Connection{
public:
    Connection(int cfd, const std::string& ip,int port); // 初始化
    ~Connection();

    int getFd(){return this->cfd;}
    std::string getIP(){return this->ip;}
    int getPort(){return this->port;}
    void setKeepAlive(bool keepAlive){this->isKeepAlive=keepAlive;}
    bool getKeepAlive(){return this->isKeepAlive;}
    bool hasData(){return writeBuffer.readableBytes()!=0;} // 判断写缓冲区中是否还有数据未写入
    ssize_t readFromFile(); // 从文件中向缓冲区读数据
    bool process(); // 处理数据
    ssize_t writeToFile(); // 向文件中写数据
private:
    int cfd; // 客户端的文件描述符
    std::string ip; // 客户端地址
    int port; // 客户端端口
    Buffer readBuffer; // 读缓冲区
    Buffer writeBuffer; // 写缓冲区
    // HTTP请求头中比较重要的信息
    bool isKeepAlive; // 是否保持长链接
};

#endif