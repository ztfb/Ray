#ifndef HTTPPROCESS
#define HTTPPROCESS

#include <memory>
#include <mutex>
#include "Buffer.h"

class HttpProcess{
public:
    static std::shared_ptr<HttpProcess> instance(); // 获取HttpParser的单例对象

    bool process(Buffer& readBuffer,Buffer& writeBuffer); // 解析http请求并进行处理
    
    HttpProcess(const HttpProcess&) = delete; // 禁用拷贝构造函数
    HttpProcess& operator=(const HttpProcess&) = delete; // 禁用赋值运算符
private:
    HttpProcess() = default; // 禁用外部构造
    // 定义一些私有方法，用于解析http请求和构造http响应
};

#endif