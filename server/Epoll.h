#ifndef EPOLL
#define EPOLL

#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <memory>

class Epoll{
public:
    static std::shared_ptr<Epoll> instance(); // 获取Epoll的单例对象
    void init(int maxSize=1024); // 初始化函数

    bool addFd(int fd,uint32_t events); // 添加文件描述符
    bool delFd(int fd); // 删除文件描述符
    bool modFd(int fd,uint32_t events); // 修改文件描述符
    int wait(int timeoutMS=-1); // 监听文件描述符的就绪，timeoutMS为超时时间
    int getFd(int index); // 获取第i个就绪的文件描述符的fd
    int getEvents(int index); // 获取第i个就绪的文件描述符的events
    
    ~Epoll(); // 析构函数释放系统资源
    Epoll(const Epoll&) = delete; // 禁用拷贝构造函数
    Epoll& operator=(const Epoll&) = delete; // 禁用赋值运算符
private:
    Epoll() = default; // 禁用外部构造
    int fd; // epoll文件描述符
    std::vector<struct epoll_event> events; // 用于保存就绪的文件描述符
};

#endif