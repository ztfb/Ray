#include "HttpProcess.h"
#include "Log.h"
#include "MySQLPool.h"
#include "RunPython.h"
#include "jsoncpp/json/json.h"

static std::shared_ptr<HttpProcess> httpProcess=nullptr;
static std::mutex mutex;

std::shared_ptr<HttpProcess> HttpProcess::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(httpProcess==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(httpProcess==nullptr){
            httpProcess=std::shared_ptr<HttpProcess>(new HttpProcess());
        }
    }
    return httpProcess;
}

bool HttpProcess::process(Connection* conn){
    // 如果处理了readBuffer，并写入了writeBuffer，则返回true
    // 如果没有处理readBuffer（一般是没有达到处理条件，例如报文不完整），则返回false

    // 在处理之前获取一个可用的连接
    /*MYSQL *mysql=NULL;
    mysql=MySQLPool::instance()->getConn();
    while(mysql==NULL){
        mysql=MySQLPool::instance()->getConn();
    }

    // 处理结束后释放该连接
    MySQLPool::instance()->freeConn(mysql);*/

    // 以下为测试程序
    /*if(readBuffer.readableBytes()<2){
        // 不足一个报文头，无法处理
        return false;
    }else{
        std::string temp="";
        std::vector<char> temp2=readBuffer.readDate(0,2);
        temp.push_back(temp2[0]);temp.push_back(temp2[1]);
        int len=std::stoi(temp);
        if(readBuffer.readableBytes()<2+len){
            // 报文不完整，无法处理
            return false;
        }else{
            std::vector<char> temp=readBuffer.getData(2+len);
            std::vector<char> message;
            for(int i=2;i<2+len;i++)message.push_back(temp[i]);
            for(int i=0;i<message.size();i++){
                if(message[i]>='a'&&message[i]<='z')message[i]=message[i]-'a'+'A';
            }
            message.push_back('\n');
            writeBuffer.appendData(message);
            return true;
        }
    }*/

    // 以下是一个python函数调用示例
    /*
    char *ret;
    PyObject *args=RunPython::instance()->initArgs(4);
    // 无论在32位机还是64位机上，指针长度和long保持一致，因此用long传递指针即可
    RunPython::instance()->buildArgs(args,0,(long)(mysql)); // 数据库连接
    RunPython::instance()->buildArgs(args,1,"get"); // 方法类型
    RunPython::instance()->buildArgs(args,2,"user/login"); // url
    RunPython::instance()->buildArgs(args,3,"{\"username\":\"test\","\password\":"\123456\"}"); // 参数
    RunPython::instance()->callFunc("prouter","router",args,ret);
    */
   
    if(conn->readBuffer.readableBytes()!=0){
        // 使用移动构造函数实现资源的转移
        std::vector<char> temp(conn->readBuffer.lookDate(0,conn->readBuffer.readableBytes()));
        conn->readBuffer.abandonData(conn->readBuffer.readableBytes()); // 不要忘记丢弃读出的数据
        log_debug("server:"+std::string(&temp[0],temp.size()));
        conn->writeBuffer.appendData(temp);
        return true;
    }else return false;
}