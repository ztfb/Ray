#include "Epoll.h"
#include "Log.h"

static std::shared_ptr<Epoll> epoll=nullptr;
static std::mutex mutex;

std::shared_ptr<Epoll> Epoll::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(epoll==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(epoll==nullptr){
            epoll=std::shared_ptr<Epoll>(new Epoll());
        }
    }
    return epoll;
}

void Epoll::init(int maxSize){
    // 创建epoll文件描述符（size没有意义，给一个大于0的数即可）
    fd=epoll_create(1024);
    // 当有文件描述符就绪时，用events保存当前就绪的文件描述符
    events.resize(maxSize);
    log_info("Epoll初始化成功...");
}

bool Epoll::addFd(int fd,uint32_t events){
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    // 添加文件描述符fd，并监听其ev事件
    int result=epoll_ctl(this->fd, EPOLL_CTL_ADD, fd, &ev);
    // 调用成功返回0
    return (result==0);
}

bool Epoll::delFd(int fd){
    epoll_event ev = {0};
    int result=epoll_ctl(this->fd, EPOLL_CTL_DEL, fd, &ev);
    return (result==0);
}

bool Epoll::modFd(int fd,uint32_t events){
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    // 将文件描述法fd的监听事件改为ev
    int result=epoll_ctl(this->fd, EPOLL_CTL_MOD, fd, &ev);
    return (result==0);
}

int Epoll::wait(int timeoutMS){
    // 监听就绪的文件描述符，并返回就绪的文件描述符数量，如果返回-1，epoll_wait()调用失败
    // timeoutMS是超时时间，传入-1表示不会超时（即如果没有就绪的文件描述符，就一直阻塞）
    // 用传入参数events保存这次就绪的文件描述法
    return epoll_wait(fd, &events[0], static_cast<int>(events.size()), timeoutMS);
}

int Epoll::getFd(int index){
    return events[index].data.fd;
}

int Epoll::getEvents(int index){
    return events[index].events;
}

Epoll::~Epoll(){
    close(fd);
}