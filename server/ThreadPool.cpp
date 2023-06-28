#include "ThreadPool.h"
#include "Log.h"

static std::shared_ptr<ThreadPool> threadPool=nullptr;
static std::mutex mutex;

std::shared_ptr<ThreadPool> ThreadPool::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(threadPool==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(threadPool==nullptr){
            threadPool=std::shared_ptr<ThreadPool>(new ThreadPool());
        }
    }
    return threadPool;
}

void ThreadPool::init(int threadNum){
    for(int i=0;i<threadNum;i++){
        // 创建threadNum个线程并设置线程分离
        std::thread([](){
            while(true){
                std::unique_lock<std::mutex> lock(ThreadPool::instance()->poolLock);
                if(!ThreadPool::instance()->taskQue.empty()){
                    // 从任务队列中取出任务并执行
                    auto task=ThreadPool::instance()->taskQue.front();
                    ThreadPool::instance()->taskQue.pop();
                    // 这里加锁和解锁的原因请看文档
                    lock.unlock(); // 暂时解锁
                    task();
                    lock.lock(); // 重新加锁
                }else ThreadPool::instance()->condvar.wait(lock); // 如果任务队列为空，则该线程阻塞
            }
        }).detach();
    }
    log_info("线程池初始化成功...");
}

void ThreadPool::addTask(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(poolLock);
    taskQue.push(task);
    condvar.notify_one(); // 唤醒一个正在阻塞的工作线程
}

ThreadPool::~ThreadPool(){
    std::unique_lock<std::mutex> lock(poolLock);
    while(!taskQue.empty()){
        // 从任务队列中取出任务并执行
        auto task=taskQue.front();
        taskQue.pop();
        task();
    }
}