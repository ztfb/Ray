#ifndef SERVER
#define SERVER

#include <string>
#include <unordered_map>

class Server{
public:
    Server(const std::string& fileName); // 服务器初始化
    void start(); // 启动服务器
private:
    void parseIni(const std::string& fileName); // 解析ini配置文件，并将解析结果写到config中
    std::unordered_map<std::string,std::string> config; // 服务器配置
};

#endif