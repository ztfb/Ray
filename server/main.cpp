#include "Server.h"

int main()
{
    Server server("config.ini"); // 创建服务器对象
    server.start(); // 启动服务器
    return 0; 
}
