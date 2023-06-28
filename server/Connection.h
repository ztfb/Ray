#ifndef CONNECTION
#define CONNECTION

#include <sys/uio.h>
#include <arpa/inet.h>
#include <string>
#include "Buffer.h"

class Connection{
public:
    Connection(int cfd, const sockaddr_in& caddr); // 初始化
    ~Connection();

    int getFd(){return this->cfd;}
    struct sockaddr_in getAddr(){return this->caddr;}
    std::string getIP(){return inet_ntoa(this->caddr.sin_addr);}
    int getPort(){return caddr.sin_port;}
    void setKeepAlive(bool keepAlive){this->isKeepAlive=keepAlive;}
    bool getKeepAlive(){return this->isKeepAlive;}

    void disconnect();
    ssize_t readFromFile(); // 从文件中向缓冲区读数据
private:
    int cfd; // 客户端的文件描述符
    struct sockaddr_in caddr; // 客户端的地址和端口信息
    Buffer readBuffer; // 读缓冲区
    Buffer writeBuffer; // 写缓冲区
    // HTTP请求头中比较重要的信息
    bool isKeepAlive; // 是否保持长链接
    int iovCnt_;
    struct iovec iov_[2];
};

#endif