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

    Epoll::instance()->init(1024); // Epoll初始
    
    RunPython::instance()->init("./temp-script"); // 初始化python运行器
    RunPython::instance()->loadModule("hello");
    RunPython::instance()->loadFunction("hello","hi");
    RunPython::instance()->loadFunction("hello","hip1");
    RunPython::instance()->loadFunction("hello","hip2");
    char *ret;
    PyObject *args=RunPython::instance()->initArgs(1);
    RunPython::instance()->buildArgs(args,0,"tom");
    // 调用无参无返回值函数
    // RunPython::instance()->callFunc("hello","hi",NULL,ret);
    // 调用有参无返回值函数
    // RunPython::instance()->callFunc("hello","hip1",args,ret);
    // 调用有参有返回值函数
    RunPython::instance()->callFunc("hello","hip2",args,ret);
    log_debug(std::string(ret));

    log_info("初始化服务器配置...");
    
}

void Server::start(){
    log_info("服务器启动成功...");
    while(true){
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