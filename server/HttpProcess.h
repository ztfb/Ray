#ifndef HTTPPROCESS
#define HTTPPROCESS

#include <memory>
#include <mutex>
#include "Connection.h"
#include "jsoncpp/json/json.h"

class Connection;

class HttpProcess{
public:
    static std::shared_ptr<HttpProcess> instance(); // 获取HttpParser的单例对象

    bool process(Connection* conn); // 解析http请求并进行处理
    
    HttpProcess(const HttpProcess&) = delete; // 禁用拷贝构造函数
    HttpProcess& operator=(const HttpProcess&) = delete; // 禁用赋值运算符
private:
    HttpProcess() = default; // 禁用外部构造
    // 定义一些私有方法，用于解析http请求和构造http响应

    // 解析成功返回true，并在该函数内丢掉readBuffer已经处理过的数据，解析失败(报文不完整，无法解析)返回false
    // 报文有语法错误也是可以成功解析并丢掉已经处理过的数据的，只要返回给客户端400错误即可
    bool httpParser(Buffer& readBuffer,Json::Value& parseResult); // 解析HTTP请求，并把解析结果放到parseResult中
    void parseLine(const std::string& line,Json::Value& parseResult); // 解析请求行
    void parseHead(const std::string& line,Json::Value& parseResult); // 解析请求头
};

#endif