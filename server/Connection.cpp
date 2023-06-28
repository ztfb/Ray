#include "Connection.h"
#include "Log.h"

Connection::Connection(int cfd, const std::string& ip,int port){
    this->cfd=cfd;
    this->ip=ip;
    this->port=port;
}

Connection::~Connection(){
    close(cfd);
}

ssize_t Connection::readFromFile(){
    ssize_t len=-1;
    while(true){
        // 边沿触发模式下，使用非阻塞文件，必须一直读到read出错为止
        // len==0表示客户端断开连接；len==-1表示无数据可读了（即数据都已经读完）
        len=readBuffer.readFromFile(cfd);
        if(len<=0)break;
    }
    return len;
}

bool Connection::process(){
    // 该函数处理readBuffer中的数据，并将处理结果写到writeBuffer中
    // 如果进行处理了，则返回true；如果没有进行处理，则返回false
}

ssize_t Connection::writeToFile(){
    // 该函数将writeBuffer中的数据写到文件中
}