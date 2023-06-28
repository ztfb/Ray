#include "Connection.h"
#include "Log.h"

Connection::Connection(int cfd, const sockaddr_in& caddr){
    this->caddr=caddr;
    this->cfd=cfd;
}

Connection::~Connection(){
    disconnect();
}

void Connection::disconnect(){
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