#ifndef HTTPPROCESS
#define HTTPPROCESS

#include <memory>
#include <mutex>
#include <map>
#include "Connection.h"

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
    bool httpParser(Buffer& readBuffer,std::map<std::string,std::string>& parseResult); // 解析HTTP请求，并把解析结果放到parseResult中
    // 根据HTTP解析结果和处理结果封装HTTP响应报文
    std::string httpBuilder(const std::string& version,const std::string& code,const std::string& connection,const std::string& body);
};

#endif