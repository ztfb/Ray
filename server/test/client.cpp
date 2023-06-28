#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string>
#include <iostream>

int main(){
    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    inet_pton(AF_INET,"192.168.186.128",&saddr.sin_addr.s_addr);
    saddr.sin_port=htons(9090);
    connect(fd,(struct sockaddr*)(&saddr),sizeof(saddr));
    char buf[2048];
    while(true){
        std::string message="I am client: "+std::to_string(getpid());
        std::string len=std::to_string(message.size()+1);//报文头
        std::string data=len+message;
        write(fd,data.c_str(),data.size()+1);
        int ret=read(fd,buf,sizeof(buf));
        if(ret==-1){
            std::cout<<"read调用失败"<<std::endl;
            break;
        }else if(ret==0){
            std::cout<<"服务器关闭"<<std::endl;
            break;
        }else if(ret>0){
            std::cout<<buf<<std::endl;
        }
    }
    close(fd);
}
