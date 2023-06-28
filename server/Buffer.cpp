#include "Buffer.h"
#include "Log.h"

Buffer::Buffer(int size){
    buffer.resize(size); // 初始化缓冲区大小
    readPos=writePos=0; // 初始化读写指针
}
int Buffer::unusedBytes(){
    return readPos;
}
int Buffer::readableBytes(){
    return writePos-readPos;
}
int Buffer::writableBytes(){
    return buffer.size()-writePos;
}
ssize_t Buffer::readFromFile(int fd){
    char temp[65535]; // 临时缓存
    // 使用分散读readv保证数据全部读完
    struct iovec iov[2];
    iov[0].iov_base=(&buffer[0])+writePos;
    iov[0].iov_len=writableBytes();
    iov[1].iov_base=temp;
    iov[1].iov_len=sizeof(temp);
    // 使用分散读将数据读到两个缓存中
    ssize_t len = readv(fd, iov, 2);
    // 读取失败返回-1
    if(len<=0)return len;
    // 读出的数据总长小于缓冲区buffer可写空间的长度
    if(len<=writableBytes()){
        writePos+=len;
    }else{
        int writable=writableBytes();
        int oldSize=buffer.size();
        if(writableBytes()+unusedBytes()>=len){
            // 可写空间加上未使用空间足够容纳从文件中读出的数据
            // 将可读空间前移，把未使用的空间用起来
            std::copy((&buffer[0])+readPos,(&buffer[0])+oldSize,&buffer[0]);
            writePos=oldSize-readPos;
            readPos=0;
            std::copy(temp,temp+len-writable,(&buffer[0])+writePos);
            writePos+=(len-writable);
        }else{
            // 容量不够时直接扩容
            buffer.resize(oldSize+len-writable+1);
            std::copy(temp,temp+len-writable,(&buffer[0])+oldSize);
            writePos=oldSize+len-writable;
        }
    }
    return len;
}
ssize_t Buffer::writeToFile(int fd){
    // 第一个可读的字节的地址是(&buffer[0])+readPos；可读字节总数为readableBytes()
    int len=write(fd,(&buffer[0])+readPos,readableBytes());
    // 返回值len是实际写入的字节数，写入失败时返回-1
    if(len<=0)return len;
    readPos+=len; // 写入成功时需要移动读指针
    return len;
}