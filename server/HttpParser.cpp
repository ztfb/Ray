#include "HttpParser.h"
#include "Log.h"

static std::shared_ptr<HttpParser> httpParser=nullptr;
static std::mutex mutex;

std::shared_ptr<HttpParser> HttpParser::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(httpParser==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(httpParser==nullptr){
            httpParser=std::shared_ptr<HttpParser>(new HttpParser());
        }
    }
    return httpParser;
}