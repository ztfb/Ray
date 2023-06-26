#ifndef TIMER
#define TIMER

#include <memory>

class Timer{
public:
    static std::shared_ptr<Timer> instance(); // 获取Timer的单例对象

    Timer(const Timer&) = delete; // 禁用拷贝构造函数
    Timer& operator=(const Timer&) = delete; // 禁用赋值运算符
private:
    Timer() = default; // 禁用外部构造
};

#endif