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

void Timer::up(int index){
    if(index==0)return;
    int parent=(index-1)/2; // 当前节点父节点的索引
    while(parent>=0){ // parent<0时，表明当前节点已经到达根节点
        if(heap[parent]<heap[index])break; // 已经满足了小根堆的特点，结束循环
        // 交换节点，并更新节点位置
        swap(parent,index);
        index=parent;
        parent=(index-1)/2;
    }
}

void Timer::down(int index){
    int child=index*2+1; // 当前节点左子树的索引
    while(child<heap.size()){ // 当前节点有左子树时
        // 应该将该节点与最小的那个子节点交换
        if(child+1<heap.size()&&heap[child+1]<heap[child])child++;
        if(heap[index]<heap[child])break; // 满足小根堆的特性，退出循环
        swap(index,child);
        index=child;
        child=index*2+1;
    }
}

void Timer::swap(int i,int j){
    std::swap(heap[i],heap[j]); // 交换位置
    // 更新索引映射
    id2index[heap[i].id]=i;
    id2index[heap[j].id]=j;
}

void Timer::pop(){
    swap(0,heap.size()-1); // 将要删除的节点先移到最后
    id2index.erase(heap.back().id); // 删除节点的映射
    heap.pop_back(); // 删除节点
    down(0); // 调整节点位置
}

void Timer::adjust(int id,int timeout){
    std::unique_lock<std::mutex> lock(timerLock);
    // 更新节点的到期时间
    heap[id2index[id]].expiration=std::chrono::high_resolution_clock::now()+std::chrono::milliseconds(timeout);
    // 调整节点位置
    down(id2index[id]);
}

void Timer::add(int id,int timeout,std::function<void()> callback){
    std::unique_lock<std::mutex> lock(timerLock);
    int index=heap.size();
    id2index[id]=index;
    // 向堆的最后加入这个节点
    heap.push_back({id,
    std::chrono::high_resolution_clock::now()+std::chrono::milliseconds(timeout),
    callback});
    // 向上调整
    up(index);
}

void Timer::clearTimeoutNode(){
    while(!heap.empty()){
        Node node=heap.front();
        // 如果根节点（它是最可能超时的节点）没有超时，说明已经没有超时节点了，退出循环
        if(std::chrono::duration_cast<std::chrono::milliseconds>(
            node.expiration-std::chrono::high_resolution_clock::now()).count()>0)break;
        node.callback(); // 执行节点超时的回调函数
        pop(); // 移出堆顶节点
    }
}

int Timer::getExpiration(){
    std::unique_lock<std::mutex> lock(timerLock);
    clearTimeoutNode(); // 先清除所有超时节点
    int result=-1;
    if(!heap.empty()) {
        result=std::chrono::duration_cast<std::chrono::milliseconds>(
            heap.front().expiration - std::chrono::high_resolution_clock::now()).count();
        if(result<0)result=0;
    }
    // 如果result==-1，说明此时没有节点了（即所有的节点都已经超时）
    // 如果result==0，说明此时有节点刚好超时，但是还没有被清除（由于服务器主循环每次都会调用该函数进行检测，因此这些刚好到期的节点下一次会被清除）
    return result;
}

void Timer::del(int id){
    std::unique_lock<std::mutex> lock(timerLock);
    if(id2index.count(id)!=0){
        int index=id2index[id];
        Node node = heap[index];
        // del中不能调用节点的回调函数，否则将会造成循环调用
        // 当节点过期或者客户端关闭时，会调用Server的disconnect函数，该函数会调用定时器的del函数
        // 如果是节点过期引起的disconnect回调，由于在调用disconnect之前，就已经清除节点，所以调用该函数无效
        // 如果是客户端关闭引起的disconnect回调，则调用del主动删除节点
        swap(index,heap.size()-1);
        id2index.erase(id); // 删除节点的映射
        heap.pop_back(); // 删除节点
        down(index); // 调整节点位置
    }
}