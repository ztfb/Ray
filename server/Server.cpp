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

Server::Server(const std::string& fileName){
    listenEvent=EPOLLRDHUP; // 需要epoll检测对端关闭事件
    connEvent=EPOLLONESHOT|EPOLLRDHUP|EPOLLET; //  保证只触发一次；需要epoll检测对端关闭事件；使用边沿触发模式
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
    // 如果mysql连接池初始化失败，将isSuccess置为false
    isSuccess=MySQLPool::instance()->init(config["mysqlHost"],std::stoi(config["mysqlPort"]),config["mysqlUsername"],
                                config["mysqlPassword"],config["mysqlDB"],std::stoi(config["sqlconnNum"]));
    
    
    Epoll::instance()->init(1024); // 初始化Epoll

    if(initSocket()==false){
        // 套接字初始化失败
        log_error("套接字初始化失败...");
        isSuccess=false;
    }else log_info("套接字初始化成功...");
    // 注意：python运行器只需要载入python路由脚本（prouter），并且给它传递若干参数：
    // 一个是MYSQL连接（在调用这个路由函数之前必须获取一个mysql连接）
    // 其他的是HTTP请求相关字符串
    // 该路由函数返回值是HTTP响应相关的字符串（主要是响应头和JSON响应体）
    // 获取一个可用的连接
    /*MYSQL *mysql=NULL;
    mysql=MySQLPool::instance()->getConn();
    while(mysql==NULL){
        mysql=MySQLPool::instance()->getConn();
    }
    RunPython::instance()->init(config["scriptPath"]); // 初始化python运行器
    RunPython::instance()->loadModule("pmysql_test");
    RunPython::instance()->loadFunction("pmysql_test","test");
    char *ret;
    PyObject *args=RunPython::instance()->initArgs(2);
    // 无论在32位机还是64位机上，指针长度和long保持一致，因此用long传递指针即可
    RunPython::instance()->buildArgs(args,0,(long)(mysql));
    RunPython::instance()->buildArgs(args,1,0);
    RunPython::instance()->callFunc("pmysql_test","test",args,ret);*/

    log_info("服务器配置初始化完成...");
    
}

void Server::start(){
    if(!isSuccess){
        log_error("服务器初始化失败...已退出...");
        return ;
    }
    log_info("Ray服务器启动...");

    while(true){
        int timeoutMS=Timer::instance()->getExpiration(); // 初始时这个函数将返回-1
        // timeoutMS==-1时，如果没有就绪事件，wait将阻塞
        int eventCount=Epoll::instance()->wait(timeoutMS);
        for(int i=0;i<eventCount;i++){
            // 处理epoll通知的就绪事件
            int fd=Epoll::instance()->getFd(i); // 获得第i个就绪事件对应的文件描述符
            uint32_t events=Epoll::instance()->getEvents(i); // 获取第i个就绪事件对应的事件类型
            if(fd==listenFd){
                // 监听套接字就绪，说明有新的客户端接入
                // listenFd是水平触发模式，无需循环检测
                struct sockaddr_in caddr;
                socklen_t len = sizeof(caddr);
                int cfd = accept(listenFd, (struct sockaddr *)&caddr, &len);
                connections.emplace((cfd,Connection(cfd,caddr))); // 创建一个连接
                log_info(connections[cfd].getIP()+":"+std::to_string(connections[cfd].getPort())+" 接入...");
                // 将cfd添加到epoll的监听文件集中
                Epoll::instance()->addFd(cfd,EPOLLIN|connEvent);
                setNonBlock(cfd); // 由于使用边沿触发模式，因此必须设置文件非阻塞
                // 将该连接添加到定时器中
                Timer::instance()->add(cfd,std::stoi(config["timeoutMS"]),
                std::bind(&Server::disconnect,this,&connections[cfd]));
            }
            // 负责和客户端通信的通信套接字就绪
            else if(events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
                // 对端出现异常，关闭该连接
                disconnect(&connections[fd]);
            }
            else if(events&EPOLLIN){
                // 读事件就绪，从文件描述符中将数据读出
                // 节点活跃，应当调整节点的到期时间
                Timer::instance()->adjust(fd,std::stoi(config["timeoutMS"]));
                ThreadPool::instance()->addTask(std::bind(
                    &Server::readEvent,this,&connections[fd]
                ));
            }
            else if(events&EPOLLOUT){
                // 写事件就绪，向文件描述法中写数据
                // 节点活跃，应当调整节点的到期时间
                Timer::instance()->adjust(fd,std::stoi(config["timeoutMS"]));
                ThreadPool::instance()->addTask(std::bind(
                    &Server::writeEvent,this,&connections[fd]
                ));
            }else log_warn("未知事件...");
        }
    }
    /*// 以下代码仅仅用于测试
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
            log_debug(buf);
        }
        const char *data="i had received!";
        write(cfd,data,strlen(data)+1);
    }
    close(cfd);*/
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
    // 构造地址和端口
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    // 网络字节序（在网络中传输的数据的字节序）是大端序，主机字节序不一定是大端序，因此要进行转换
    saddr.sin_addr.s_addr=htonl(INADDR_ANY); // 绑定本机所有地址
    saddr.sin_port=htons(std::stoi(config["port"])); // 将port从主机字节序转为网络字节序
    // 设置优雅关闭
    struct linger optLinger={0};
    optLinger.l_onoff=1;
    optLinger.l_linger=1;
    int ret = setsockopt(listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret==-1){
        log_error("设置优雅关闭失败...");
        return false;
    }
    // 设置端口复用
    int optval = 1;
    ret = setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret==-1){
        log_error("设置端口复用失败...");
        return false;
    }
    // 绑定地址和端口
    ret=bind(listenFd,(struct sockaddr*)(&saddr),sizeof(saddr));
    if(ret==-1){
        log_error("监听套接字绑定失败...");
        return false;
    }
    // 监听套接字
    ret=listen(listenFd,8); // 监听套接字
    if(ret==-1){
        log_error("监听套接字失败...");
        return false;
    }
    // 还需要监听【listenFd】上的读事件
    // 一旦有客户端通过三次握手建立连接，就会触发listenFd上的可读事件
    Epoll::instance()->addFd(listenFd,listenEvent|EPOLLIN);
    // 设置监听套接字为非阻塞
    setNonBlock(listenFd);
    return true;
}

void Server::setNonBlock(int fd){
    int flag=fcntl(fd, F_GETFD);
    flag|=O_NONBLOCK;
    fcntl(fd,F_SETFL,flag);
}

void Server::disconnect(Connection* conn){
    log_info(conn->getIP()+":"+std::to_string(conn->getPort())+" 离开...");
    Epoll::instance()->delFd(conn->getFd());
    conn->disconnect();
}

void Server::readEvent(Connection* conn){
    int ret=conn->readFromFile();
    if(ret==0){ // 客户端端开连接
        disconnect(conn);
        return ;
    }
    process(conn);
}

void Server::process(Connection* conn){

}

void Server::writeEvent(Connection* conn){

}