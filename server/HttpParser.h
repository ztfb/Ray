#ifndef HTTPPARSER
#define HTTPPARSER

#include <memory>
#include <mutex>

class HttpParser{
public:
    static std::shared_ptr<HttpParser> instance(); // 获取HttpParser的单例对象

    HttpParser(const HttpParser&) = delete; // 禁用拷贝构造函数
    HttpParser& operator=(const HttpParser&) = delete; // 禁用赋值运算符
private:
    HttpParser() = default; // 禁用外部构造
};

#endif