#ifndef LOG
#define LOG

#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

class Log{
public:
    // 在开发服务器核心时，最好使用同步的日志系统；服务器上线运行时，可以使用异步的日志系统
    static std::shared_ptr<Log> instance(); // 获取Log的单例对象
    void init(bool isOpenLog,int logLevel,bool isAsync); // 初始化Log

    bool open(){return isOpenLog;}
    int level(){return logLevel;}
    bool async(){return isAsync;}
    int currSize(){return logQue.size();}
    std::mutex& lock(){return queLock;}
    std::condition_variable& cond(){return condvar;}

    void addLog(const std::string& log){Log::instance()->logQue.push(log);} // 将日志信息添加到日志队列中

    ~Log();
    Log(const Log&) = delete; // 禁用拷贝构造函数
    Log& operator=(const Log&) = delete; // 禁用赋值运算符
private:
    Log() = default; // 禁用外部构造
    bool isOpenLog; // 是否打开日志系统
    int logLevel; // 当前日志级别
    bool isAsync; // 是否使用异步输出
    std::queue<std::string> logQue; // 日志队列
    std::mutex queLock; // 队列互斥锁
    std::condition_variable condvar; // 条件变量
};

void log_debug(const std::string& log);
void log_info(const std::string& log);
void log_warn(const std::string& log);
void log_error(const std::string& log);

#endif