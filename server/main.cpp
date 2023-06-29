#include "Server.h"
#include <iostream>

int main(int argc,char *argv[])
{
    if(argc!=3){
        // 如果提供的参数和需要的不匹配，则使用默认的参数
        Server server; // 创建服务器对象
        server.start(); // 启动服务器
    }else{
        // argv[1]是配置文件路径参数
        // argv[2]是脚本文件路径参数
        Server server(argv[1],argv[2]); // 创建服务器对象
        server.start(); // 启动服务器
    }
    return 0; 
}
