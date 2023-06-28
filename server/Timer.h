#ifndef TIMER
#define TIMER

#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>

struct Node{
    // 定时器的堆中存储的节点的结构
    int id; // 节点的id（对于文件而言，id可以是文件描述符）
    // 到期时间点
    std::chrono::high_resolution_clock::time_point expiration;
    std::function<void()> callback; // 节点到期后需要触发的回调函数
    bool operator<(const Node& node) {
        return expiration<node.expiration;
    }
};

class Timer{
public:
    static std::shared_ptr<Timer> instance(); // 获取Timer的单例对象

    void adjust(int id,int timeout); // 更新一个节点的到期时间
    void add(int id,int timeout,std::function<void()> callback); //添加一个节点
    int getExpiration(); // 获取距离最近的到期时间的毫秒数
    // 用于客户端主动断开连接时从定时器中删除一个节点
    void del(int id); // 根据id删除一个节点

    Timer(const Timer&) = delete; // 禁用拷贝构造函数
    Timer& operator=(const Timer&) = delete; // 禁用赋值运算符
private:
    Timer() = default; // 禁用外部构造
    void up(int index); // 将heap中索引为index的节点上移，保证小根堆的结构
    void down(int index); // 将heap中索引为index的节点下移，保证小根堆的结构
    void swap(int i,int j); // 将heap中索引为i和j的两个节点交换位置
    void pop(); // 取出堆顶节点
    void clearTimeoutNode(); // 清除所有超时的节点

    std::mutex timerLock; // 定时器互斥锁
    std::vector<Node> heap; // 定时器堆（是一个小根堆，顶部节点是到期时间最近的节点）
    std::unordered_map<int,int> id2index; // 节点id到节点在heap中索引的映射
};

#endif