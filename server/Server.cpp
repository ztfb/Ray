#include "Server.h"
#include <fstream>
#include "Log.h"

Server::Server(const std::string& fileName){
    system("clear"); // 清屏，方便之后的日志输出

    parseIni(fileName); // 解析配置文件
    Log::instance()->init(
        (config["isOpenLog"]=="true"?true:false),
        std::stoi(config["logLevel"]),
        (config["isAsync"]=="true"?true:false)
    ); // 初始化日志系统
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