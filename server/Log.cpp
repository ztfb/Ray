#include "Log.h"
#include <unistd.h>
#include <iostream>
#include <ctime>

static std::shared_ptr<Log> log=nullptr;
static std::mutex mutex;

std::shared_ptr<Log> Log::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(log==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
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
    if(this->isOpenLog&&this->isAsync){
        // 如果开启了日志系统并且是异步的，则需要创建一个子线程负责写日志
        std::thread([](){
            while(true){
                std::unique_lock<std::mutex> lock(Log::instance()->queLock); // 多线程操作STL需要加锁
                if(!Log::instance()->logQue.empty()){
                    std::cout<<Log::instance()->logQue.front()<<std::endl;
                    Log::instance()->logQue.pop();
                }else{
                    Log::instance()->condvar.wait(lock); // 线程需要重新加锁判断条件是否满足，故需要配合mutex使用
                }
            }
        }).detach(); // 设置线程分离
    }
    log_info("日志系统初始化成功...");
}

void log_base(const std::string& log,int level1,const std::string& level2){
    if(Log::instance()->open()){ // 日志系统打开的情况下才能进行输出
        if(Log::instance()->level()<=level1){ // 当前使用的日志级别不超过level1，level1级别的日志才能输出
            std::string message;
            time_t now;
            time(&now); //获取1970年1月1日0点0分0秒到现在经过的秒数
            tm *p=localtime(&now); //将秒数转换为本地时间
            message.append(std::to_string(p->tm_year+1900)+"-"+std::to_string(p->tm_mon+1)+"-"+std::to_string(p->tm_mday)+" "
                            +std::to_string(p->tm_hour)+":"+std::to_string(p->tm_min)+":"+std::to_string(p->tm_sec));
            message.append(" [tid: "+std::to_string(gettid())+"]");
            message.append(" ["+level2+"] ");
            message.append(log);
            if(Log::instance()->async()){
                // 异步情况下，将日志加入到日志队列中
                std::unique_lock<std::mutex> lock(Log::instance()->lock()); // 多线程操作STL需要加锁
                Log::instance()->addLog(message);
                // 如果有线程在等待，则通知它可以解除等待（如果没有线程在等待，该方法相当于没有调用）
                Log::instance()->cond().notify_one();
            }else{
                // 同步情况下，直接输出日志
                std::cout<<message<<std::endl;
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

Log::~Log(){
    std::unique_lock<std::mutex> lock(queLock);
    while(!logQue.empty()){
        std::cout<<logQue.front()<<std::endl;
        logQue.pop();
    }
}