#include "Timer.h"
#include "Log.h"

static std::shared_ptr<Timer> timer=nullptr;
static std::mutex mutex;

std::shared_ptr<Timer> Timer::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(timer==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(timer==nullptr){
            timer=std::shared_ptr<Timer>(new Timer());
        }
    }
    return timer;
}