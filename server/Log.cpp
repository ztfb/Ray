#include "Log.h"
#include <iostream>
#include <ctime>

static std::shared_ptr<Log> log=nullptr;
static std::mutex mutex;

std::shared_ptr<Log> Log::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(log==nullptr){
        std::lock_guard<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(log==nullptr){
            log=std::shared_ptr<Log>(new Log());
        }
    }
    return log;
}

void Log::init(bool isOpenLog,int logLevel,bool isAsync){
    this->isOpenLog=isOpenLog;
    // debug:1 info:2 warn:3 error:4
    this->logLevel=logLevel;
    this->isAsync=isAsync;
}

void log_base(const std::string& log,int level1,const std::string& level2){
    if(Log::instance()->open()){ // 日志系统打开的情况下才能进行输出
        if(Log::instance()->level()<=level1){ // 当前使用的日志级别不超过level1，level1级别的日志才能输出
            std::string log;
            time_t now;
            time(&now); //获取1970年1月1日0点0分0秒到现在经过的秒数
            tm *p=localtime(&now); //将秒数转换为本地时间
            log.append(std::to_string(p->tm_year+1900)+"-"+std::to_string(p->tm_mon+1)+"-"+std::to_string(p->tm_mday)+" "
                            +std::to_string(p->tm_hour)+":"+std::to_string(p->tm_min)+":"+std::to_string(p->tm_sec));
            log.append(" ["+level2+"] ");
            log.append(log);
            if(Log::instance()->async()){
                // 异步情况下，将日志加入到日志队列中
                Log::instance()->addLog(log);
            }else{
                // 同步情况下，直接输出日志
                std::cout<<log<<std::endl;
            }
        }
    }
}
void log_debug(const std::string& log){
    log_base(log,1,"debug");
}
void log_info(const std::string& log){
    log_base(log,2,"info");
}
void log_warn(const std::string& log){
    log_base(log,3,"warn");
}
void log_error(const std::string& log){
    log_base(log,4,"error");
}