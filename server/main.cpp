#include "Server.h"
#include <iostream>

int main(int argc,char *argv[])
{
    if(argc!=3){
        std::cout<<"提供的参数个数和需要的参数个数不匹配...无法启动服务器..."<<std::endl;
    }else{
        // argv[1]是配置文件路径参数，由ui生成并提供
        // argv[2]是脚本文件路径参数，由ui生成并提供
        Server server(argv[1],argv[2]); // 创建服务器对象
        server.start(); // 启动服务器
    }
    return 0; 
}