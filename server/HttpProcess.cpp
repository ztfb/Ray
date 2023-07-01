#include "HttpProcess.h"
#include "Log.h"
#include "MySQLPool.h"
#include "RunPython.h"
#include <regex>
#include <iostream>
#include <map>

static std::shared_ptr<HttpProcess> httpProcess=nullptr;
static std::mutex mutex;
static std::map<std::string,std::string> codes={
    std::pair<std::string,std::string>("200","OK"),
    std::pair<std::string,std::string>("400","Bad Request"),
    std::pair<std::string,std::string>("403","Forbidden"),
    std::pair<std::string,std::string>("404","Not Found"),
    std::pair<std::string,std::string>("405","Method Not Allowed"),
    std::pair<std::string,std::string>("406","Not Acceptable"),
    std::pair<std::string,std::string>("500","Internal Server Error"),
    std::pair<std::string,std::string>("505","HTTP Version Not Supported")
};

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
   
    Json::Value parseResult;
    parseResult["error"]=false;
    if(httpParser(conn->readBuffer,parseResult)){
        std::string response;
        if(parseResult["error"]==true){
            // HTTP请求存在语法错误，不移交上层，直接返回400错误报文
            response=httpBuilder(parseResult["version"].asString(),"400",parseResult["connection"].asString(),"");
        }else if(parseResult["version"].asString()!="1.0"&&parseResult["version"].asString()!="1.1"){
            // 不支持1.0，1.1以外的HTTP协议版本，不移交上层，直接返回505错误报文
            response=httpBuilder(parseResult["version"].asString(),"505",parseResult["connection"].asString(),"");
        }else if(parseResult["method"].asString()!="GET"&&parseResult["method"].asString()!="POST"){
            // 不支持GET POST以外的方法，不移交上层，直接返回405错误报文
            response=httpBuilder(parseResult["version"].asString(),"405",parseResult["connection"].asString(),"");
        }else if(!parseResult["content-type"].asString().empty()&&parseResult["content-type"].asString()!="application/json"){
            // 不支持json格式以外的数据，不移交上层，直接返回406错误报文
            response=httpBuilder(parseResult["version"].asString(),"406",parseResult["connection"].asString(),"");
        }else{
            // 解析成功，交给上层python脚本进行处理
            if(parseResult["connection"]=="keep-alive")conn->setKeepAlive(true); // 设置长连接
            // 处理结束，根据处理结果构造HTTP响应报文
        }
        std::vector<char> data(response.begin(),response.end());
        conn->writeBuffer.appendData(data);
        return true; // 解析并处理完成，返回true，向客户端发送响应报文
    }else return false; // 解析失败，报文不完整，等待后面报文到达后继续解析
}

bool HttpProcess::httpParser(Buffer& readBuffer,Json::Value& parseResult){
    int currLen=0; // 当前已经解析过的数据的长度
    std::string currLine; // 当前行内容
    int state=0; // 当前的解析状态 0：正在解析请求行；1：正在解析请求头
    while(true){
        if(readBuffer.readableBytes()<1) return false; // 数据不足，解析失败
        char c=readBuffer.lookDate(currLen,currLen+1).at(0); // 取出一个字节的数据
        currLen++;
        if(c=='\r'){
            if(readBuffer.readableBytes()<1) return false; // 数据不足，解析失败
            char cc=readBuffer.lookDate(currLen,currLen+1).at(0); // 取出一个字节的数据
            if(cc='\n'){// 解析到一个行的末尾
                currLen++;
                switch (state){
                case 0:{
                    // 解析请求行
                    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$"); // 请求行的正则表达式
                    std::smatch subMatch;
                    if(regex_match(currLine, subMatch, patten)){   
                        parseResult["method"]=std::string(subMatch[1]); // 请求方法
                        parseResult["url"]=std::string(subMatch[2]); // url
                        parseResult["version"]=std::string(subMatch[3]); // HTTP版本
                    }else parseResult["error"]=true;; // 匹配失败，存在语法错误，设置parseResult的错误标志
                    state=1; // 下一步需要解析请求头
                    break;
                }
                case 1:{
                    // 解析请求头
                    std::regex patten("^([^:]*) ?: ?(.*)$");
                    std::smatch subMatch;
                    if(regex_match(currLine, subMatch, patten)){
                        std::string key=subMatch[1];
                        {
                            // 将key的单词的首字母统一转小写
                            if(key[0]>='A'&&key[0]<='Z')key[0]=key[0]-'A'+'a';
                            int index=1;for(;index<key.size();index++)if(key[index]=='-')break;
                            if(index+1<key.size()&&key[index+1]>='A'&&key[index+1]<='Z')key[index+1]=key[index+1]-'A'+'a';
                        }
                        parseResult[key]=std::string(subMatch[2]);
                    }else parseResult["error"]=true;; // 匹配失败，存在语法错误，设置parseResult的错误标志
                    // 检查是否能继续解析请求体
                    if(readBuffer.readableBytes()<2) return false; // 数据不足，解析失败
                    std::vector<char> temp=readBuffer.lookDate(currLen,currLen+2);
                    if(temp[0]=='\r'&&temp[1]=='\n'){ // 出现换行，开始解析请求体
                        currLen+=2;
                        // 如果content-length不存在，则返回空字符串
                        std::string str=parseResult["content-length"].asString();
                        int bodyLen=0;
                        if(!str.empty())bodyLen=std::stoi(str);
                        if(readBuffer.readableBytes()<bodyLen) return false; // 数据不足，解析失败
                        std::vector<char> body=readBuffer.lookDate(currLen,currLen+bodyLen);
                        currLen+=bodyLen;
                        readBuffer.abandonData(currLen); // 丢弃掉已经处理过的数据
                        parseResult["body"]=std::string(body.begin(),body.end());
                        return true; // 解析成功
                    }
                    break;
                }
                }
                currLine=""; // 解析完当前行后清空行
            }else currLine.push_back(c);
        }else currLine.push_back(c);
    }
}

std::string HttpProcess::httpBuilder(const std::string& version,const std::string& code,const std::string& connection,const std::string& body){
    std::string response;
    response="HTTP/"+version+" "+code+" "+codes[code]+"\r\n"; // 首行
    if(connection=="keep-alive")response.append("Connection: keep-alive\r\n");
    response.append("Content-Type: application/json\r\n");
    response.append("Content-Length: "+std::to_string(body.size())+"\r\n");
    response.append("Access-Control-Allow-Methods: GET,POST\r\n");
    response.append("\r\n");
    response.append(body);
    return response;
}