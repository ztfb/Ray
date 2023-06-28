#include "Server.h"
#include "Log.h"
#include "ThreadPool.h"
#include "Buffer.h"
#include "MySQLPool.h"
#include "Epoll.h"
#include "Timer.h"
#include "RunPython.h"
#include <fstream>
#include <functional>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
int cfd;
Server::Server(const std::string& fileName){
    system("clear"); // 清屏，方便之后的日志输出
    parseIni(fileName); // 解析配置文件

    // 初始化日志系统
    Log::instance()->init(
        (config["isOpenLog"]=="true"?true:false),
        std::stoi(config["logLevel"]),
        (config["isAsync"]=="true"?true:false)
    );

    ThreadPool::instance()->init(std::stoi(config["threadNum"])); // 初始化线程池
    
    // 初始化MySQL连接池
    MySQLPool::instance()->init(config["mysqlHost"],std::stoi(config["mysqlPort"]),config["mysqlUsername"],
                                config["mysqlPassword"],config["mysqlDB"],std::stoi(config["sqlconnNum"]));
    
    
    Epoll::instance()->init(1024); // 初始化Epoll

    if(initSocket()==false){
        // 套接字初始化失败
        log_error("套接字初始化失败...");
        isSuccess=false;
    }

    // 初始化客户端套接字（仅仅用于测试）
    // 把cfd定义为全局变量
    struct sockaddr_in caddr;
    int len=sizeof(caddr);
    cfd=accept(listenFd,(struct sockaddr *)(&caddr),(socklen_t*)&len);
    // 注意：python运行器只需要载入python路由脚本（prouter），并且给它传递若干参数：
    // 一个是MYSQL连接（在调用这个路由函数之前必须获取一个mysql连接）
    // 其他的是HTTP请求相关字符串
    // 该路由函数返回值是HTTP响应相关的字符串（主要是响应头和JSON响应体）
    // 获取一个可用的连接
    /*MYSQL *mysql=NULL;
    mysql=MySQLPool::instance()->getConn();
    while(mysql==NULL){
        mysql=MySQLPool::instance()->getConn();
    }*/
    /*RunPython::instance()->init("./temp-script"); // 初始化python运行器
    RunPython::instance()->loadModule("pmysql_test");
    RunPython::instance()->loadFunction("pmysql_test","test");
    char *ret;
    PyObject *args=RunPython::instance()->initArgs(2);
    // 无论在32位机还是64位机上，指针长度和long保持一致，因此用long传递指针即可
    RunPython::instance()->buildArgs(args,0,(long)(mysql));
    RunPython::instance()->buildArgs(args,1,3);
    RunPython::instance()->callFunc("pmysql_test","test",args,ret);*/

    log_info("服务器配置初始化完成...");
    
}

void Server::start(){
    if(!isSuccess){
        log_error("服务器初始化失败...已退出...");
        return ;
    }
    log_info("Ray服务器启动...");

    // 以下代码仅仅用于测试
    char buf[2048];
    while(true){
        int num=read(cfd,buf,sizeof(buf)); // 阻塞函数
        if(num==-1){
            log_error("read函数调用失败...");
            break;
        }else if(num==0){
            log_warn("客户端断开连接...");
            break;
        }else if(num>0){
            std::cout<<buf<<std::endl;
        }
        const char *data="i had received!";
        write(cfd,data,strlen(data)+1);
    }
    close(cfd);
}

Server::~Server(){
    // 析构函数中释放系统资源
    close(listenFd);
}

void Server::parseIni(const std::string& fileName){
    // 初始化config
    config.insert({
        {"port","9090"},
        {"timeoutMS","60000"},
        {"mode","3"},
        {"isOptLinger","true"},
        {"threadNum","4"},
        {"isOpenLog","true"},
        {"logLevel","1"},
        {"logQueSize","1024"}
    });
    std::ifstream file;
    file.open(fileName,std::ios::in);
    if(!file.is_open()){
        return ;
    }
    // 逐行读取配置文件并解析
    std::string buf;
    while (getline(file,buf)){
        if(buf[0]=='#')continue; // 跳过注释行
        else{ // 解析键值对
            int index=0;
            for(;index<buf.size();index++){
                if(buf[index]=='=')break; // 找到 = 所在位置
            }
            config[buf.substr(0,index)]=buf.substr(index+1,buf.size()-index-1);
        }
    }
    file.close();
}

bool Server::initSocket(){
    // 在linux系统中，将套接字抽象为文件，可以使用文件描述符引用
    // 对于用于监听的套接字，读文件可以取出成功建立连接的客户端的通信套接字
    // 对于用于通信的套接字，读写文件可以实现与网络中其他进程的通信
    listenFd=socket(AF_INET,SOCK_STREAM,0); // 创建用于监听的套接字（使用IPv4的TCP协议）
    if(listenFd==-1){
        log_error("监听套接字创建失败...");
        return false; // 创建套接字失败
    }

    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    // 网络字节序（在网络中传输的数据的字节序）是大端序，主机字节序不一定是大端序，因此要进行转换
    saddr.sin_addr.s_addr=htonl(INADDR_ANY); // 绑定本机所有地址
    saddr.sin_port=htons(std::stoi(config["port"])); // 将port从主机字节序转为网络字节序
    int ret=bind(listenFd,(struct sockaddr*)(&saddr),sizeof(saddr));
    if(ret==-1){
        log_error("监听套接字绑定失败...");
        return false;
    }

    ret=listen(listenFd,8); // 监听套接字
    if(ret==-1){
        log_error("监听套接字失败...");
        return false;
    }

    return true;
}

void Server::setNonBlock(int fd){
    int flag=fcntl(fd, F_GETFD);
    flag|=O_NONBLOCK;
    fcntl(fd,F_SETFL,flag);
}