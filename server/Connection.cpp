#include "Connection.h"
#include "Log.h"
#include "MySQLPool.h"

int Connection::connNum=0; // 初始化
Connection::Connection(int cfd, const std::string& ip,int port){
    connNum++;
    this->cfd=cfd;
    this->ip=ip;
    this->port=port;
    // 默认打开keep-alive
    isKeepAlive=true;
}

Connection::~Connection(){
    connNum--;
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
    // 如果进行处理了，则返回true；如果没有进行处理，则返回false（如果只返回true或false将导致无法继续处理）
    // 在处理之前获取一个可用的连接
    /*MYSQL *mysql=NULL;
    mysql=MySQLPool::instance()->getConn();
    while(mysql==NULL){
        mysql=MySQLPool::instance()->getConn();
    }

    // 处理结束后释放该连接
    MySQLPool::instance()->freeConn(mysql);*/


    // 以下为测试程序
    /*if(readBuffer.readableBytes()<2){
        // 不足一个报文头，无法处理
        return false;
    }else{
        std::string temp="";
        std::vector<char> temp2=readBuffer.readDate(0,2);
        temp.push_back(temp2[0]);temp.push_back(temp2[1]);
        int len=std::stoi(temp);
        if(readBuffer.readableBytes()<2+len){
            // 报文不完整，无法处理
            return false;
        }else{
            std::vector<char> temp=readBuffer.getData(2+len);
            std::vector<char> message;
            for(int i=2;i<2+len;i++)message.push_back(temp[i]);
            for(int i=0;i<message.size();i++){
                if(message[i]>='a'&&message[i]<='z')message[i]=message[i]-'a'+'A';
            }
            message.push_back('\n');
            writeBuffer.appendData(message);
            return true;
        }
    }*/
    if(readBuffer.readableBytes()!=0){
        // 使用移动构造函数实现资源的转移
        std::vector<char> temp(readBuffer.lookDate(0,readBuffer.readableBytes()));
        readBuffer.abandonData(readBuffer.readableBytes()); // 不要忘记丢弃读出的数据
        log_debug(std::string(&temp[0],temp.size()));
        writeBuffer.appendData(temp);
        return true;
    }else return false;
}

ssize_t Connection::writeToFile(){
    // 该函数将writeBuffer中的数据写到文件中

    // 以下为测试程序
    ssize_t len=writeBuffer.writeToFile(cfd);
    return len;
}