#ifndef BUFFER
#define BUFFER

#include <vector>
#include <string>
#include <atomic>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class Buffer{
public:
    Buffer(int size=1024);
    int unusedBytes(); // 目前没有被使用的字节数
    int readableBytes(); // 可读字节数
    int writableBytes(); // 可写字节数
    
    ssize_t readFromFile(int fd); // 从文件fd中读数据到可写空间
    ssize_t writeToFile(int fd); // 从可读空间向文件fd中写数据
    void appendData(const std::vector<char>& data); // 向缓冲区可写空间中添加数据
    std::vector<char> readDate(int begin,int end); // 读缓冲区可读空间[begin,end)中的数据（并非真正取出）
    std::vector<char> getData(int len); // 将缓冲区可读空间中的len字节数据真正取出
private:
    std::vector<char> buffer; // 缓冲区（本质是字节序列）
    // 0～readPos：暂时没有被使用的空间
    // readPos～writePos：可以读的空间（可以把这部分数据读到文件中）
    // writePos～buffer.size：可以写的空间（可以将文件中的数据写到这部分空间中）
    std::atomic<int> readPos; // 读指针
    std::atomic<int> writePos; // 写指针
};

#endif