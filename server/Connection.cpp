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

    // 以下为测试程序
    std::string data=readBuffer.getData();// 先把数据全部读出
    log_debug(data);
    if(data.size()<2){
        // 不足一个报文头，无法处理
        readBuffer.appendData(data);
        return false;
    }else{
        int len=std::stoi(data.substr(0,2));
        if(data.size()<2+len){
            // 报文不完整，无法处理
            readBuffer.appendData(data);
            return false;
        }else{
            std::string message=data.substr(2,len);
            for(int i=0;i<message.size();i++){
                if(message[i]>='a'&&message[i]<='z')message[i]=message[i]-'a'+'A';
            }
            message+='\n';
            writeBuffer.appendData(message);
            readBuffer.appendData(data.substr(2+len,data.size()-2-len));
            return true;
        }
    }
}

ssize_t Connection::writeToFile(){
    // 该函数将writeBuffer中的数据写到文件中

    // 以下为测试程序
    ssize_t len=writeBuffer.writeToFile(cfd);
    return len;
}