#include "Server.h"
#include "Log.h"
#include "ThreadPool.h"
#include "Buffer.h"
#include "MySQLPool.h"
#include "Epoll.h"
#include "Timer.h"
#include <fstream>
#include <functional>
#include <unistd.h>

void test(int a,int b){
    sleep(3);
    while (true)
    {
        log_debug(std::to_string(a+b));
    }
}

Server::Server(const std::string& fileName){
    system("clear"); // 清屏，方便之后的日志输出

    parseIni(fileName); // 解析配置文件
    Log::instance()->init(
        (config["isOpenLog"]=="true"?true:false),
        std::stoi(config["logLevel"]),
        (config["isAsync"]=="true"?true:false)
    ); // 初始化日志系统

    ThreadPool::instance()->init(std::stoi(config["threadNum"])); // 初始化线程池
    ThreadPool::instance()->addTask(std::bind(test,0,1));
    ThreadPool::instance()->addTask(std::bind(test,0,2));
    
    // 初始化MySQL连接池
    MySQLPool::instance()->init(config["mysqlHost"],std::stoi(config["mysqlPort"]),config["mysqlUsername"],
                                config["mysqlPassword"],config["mysqlDB"],std::stoi(config["sqlconnNum"]));
    
    // 测试
    MYSQL *conn=MySQLPool::instance()->getConn();
    const char * sql="insert into t1 values(4)";
    if(conn!=nullptr){
        mysql_query(conn,sql);
        MySQLPool::instance()->freeConn(conn);
    }

    // Buffer测试
    int fd1 = open("./file1",O_RDONLY,0600);
    int fd2 = open("./file2",O_WRONLY,0600);
    int fd3 = open("./file3",O_RDONLY,0600);
    int fd4 = open("./file4",O_WRONLY,0600);
    Buffer buffer;
    log_debug(std::to_string(buffer.readFromFile(fd1)));
    log_debug(std::to_string(buffer.writeToFile(fd2)));
    log_debug(std::to_string(buffer.readFromFile(fd3)));
    log_debug(std::to_string(buffer.writeToFile(fd4)));
    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);

    // Epoll初始化
    Epoll::instance()->init(1024);
    
    log_info("初始化服务器配置...");
    
}

void Server::start(){
    log_info("服务器启动成功...");
    sleep(3);
    while(true){
        log_debug("3");
    }
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