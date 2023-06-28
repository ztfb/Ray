#ifndef THREADPOOL
#define THREADPOOL

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <memory>

class ThreadPool{
public:
    static std::shared_ptr<ThreadPool> instance(); // 获取ThreadPool的单例对象
    void init(int threadNum); // 初始化线程池
    void addTask(std::function<void()> task); //向任务队列中添加任务

    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete; // 禁用拷贝构造函数
    ThreadPool& operator=(const ThreadPool&) = delete; // 禁用赋值运算符
private:
    ThreadPool() = default; // 禁用外部构造
    std::mutex poolLock; // 线程池互斥锁
    std::condition_variable condvar; // 条件变量
    std::queue<std::function<void()>> taskQue; // 任务队列
};

#endif