#include "HttpProcess.h"
#include "Log.h"
#include "MySQLPool.h"
#include "RunPython.h"
#include <regex>
#include <iostream>

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
   
    std::map<std::string,std::string> parseResult;
    parseResult["error"]="false";
    if(httpParser(conn->readBuffer,parseResult)){
        std::string response;
        if(parseResult["error"]=="true"){
            // HTTP请求存在语法错误，不移交上层，直接返回400错误报文
            response=httpBuilder(parseResult["version"],"400",parseResult["connection"],"");
        }else if(parseResult["version"]!="1.0"&&parseResult["version"]!="1.1"){
            // 不支持1.0，1.1以外的HTTP协议版本，不移交上层，直接返回505错误报文
            response=httpBuilder(parseResult["version"],"505",parseResult["connection"],"");
        }else if(parseResult["method"]!="GET"&&parseResult["method"]!="POST"){
            // 不支持GET POST以外的方法，不移交上层，直接返回405错误报文
            response=httpBuilder(parseResult["version"],"405",parseResult["connection"],"");
        }else if(!parseResult["content-type"].empty()&&parseResult["content-type"]!="application/json"){
            // 不支持json格式以外的数据，不移交上层，直接返回406错误报文
            response=httpBuilder(parseResult["version"],"406",parseResult["connection"],"");
        }else{
            if(parseResult["method"]=="GET"){
                // 如果是GET请求，则需要重写parseResult的url和body
                // 将纯url的部分写到url中，参数部分写到body中
                std::regex p1("^([^\\?]*)\\?([^]*)$");std::smatch url;regex_match(parseResult["url"],url,p1);
                parseResult["url"]=url[1];
                std::string params(url[2]); // 参数部分
                // 构建关于参数的json字符串，并写道parseResult["body"]中
                parseResult["body"]="{";
                std::string temp;
                for(int i=0;i<params.size();i++){
                    if(params[i]=='='){
                        parseResult["body"].append("\""+temp+"\":");
                        temp="";
                    }else if(params[i]=='&'){
                        parseResult["body"].append("\""+temp+"\",");
                        temp="";
                    }else temp.push_back(params[i]);
                }
                parseResult["body"].append("\""+temp+"\"}");
            }
            // 解析成功，交给上层python脚本进行处理
            
            if(parseResult["connection"]=="keep-alive")conn->setKeepAlive(true); // 设置长连接

            // 在处理之前获取一个可用的连接
            MYSQL *mysql=NULL;
            mysql=MySQLPool::instance()->getConn();
            while(mysql==NULL){
                mysql=MySQLPool::instance()->getConn();
            }

            // 调用python路由函数
            char *ret=nullptr;
            PyObject *args=RunPython::instance()->initArgs(4);
            // 无论在32位机还是64位机上，指针长度和long保持一致，因此用long传递指针即可
            RunPython::instance()->buildArgs(args,0,(long)(mysql)); // 数据库连接
            RunPython::instance()->buildArgs(args,1,parseResult["method"].c_str()); // 方法类型
            RunPython::instance()->buildArgs(args,2,parseResult["url"].c_str()); // url
            RunPython::instance()->buildArgs(args,3,parseResult["body"].c_str()); // 参数
            RunPython::instance()->callFunc("prouter","router",args,ret);

            // 处理结束后释放该连接
            MySQLPool::instance()->freeConn(mysql);

            // 处理结束，根据处理结果构造HTTP响应报文
            if(ret==nullptr){ // 函数调用失败，服务器内部错误
                response=httpBuilder(parseResult["version"],"500",parseResult["connection"],"");
            }else{
                std::string body(ret);
                if(body=="404"){ // 返回404，说明没有找到方法
                    response=httpBuilder(parseResult["version"],"404",parseResult["connection"],"");
                }else response=httpBuilder(parseResult["version"],"200",parseResult["connection"],body);
            }
        }
        std::vector<char> data(response.begin(),response.end());
        conn->writeBuffer.appendData(data);
        return true; // 解析并处理完成，返回true，向客户端发送响应报文
    }else return false; // 解析失败，报文不完整，等待后面报文到达后继续解析
}

bool HttpProcess::httpParser(Buffer& readBuffer,std::map<std::string,std::string>& parseResult){
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
                        parseResult["method"]=subMatch[1]; // 请求方法
                        parseResult["url"]=subMatch[2]; // url
                        parseResult["version"]=subMatch[3]; // HTTP版本
                    }else{
                        parseResult["error"]="true";; // 匹配失败，存在语法错误，设置parseResult的错误标志
                    }
                    state=1; // 下一步需要解析请求头
                    break;
                }
                case 1:{
                    // 正则表达式解析请求头
                    std::regex patten("^([^:]*) ?: ?(.*)$");
                    std::smatch subMatch;
                    if(regex_match(currLine, subMatch, patten)){
                        std::string key=subMatch[1];
                        {
                            // 将key的单词的首字母统一转小写，方便处理
                            // 例如将头部Content-Type转为content-type
                            if(key[0]>='A'&&key[0]<='Z')key[0]=key[0]-'A'+'a';
                            int index=1;for(;index<key.size();index++)if(key[index]=='-')break;
                            if(index+1<key.size()&&key[index+1]>='A'&&key[index+1]<='Z')key[index+1]=key[index+1]-'A'+'a';
                        }
                        parseResult[key]=subMatch[2];
                    }else parseResult["error"]="true";; // 匹配失败，存在语法错误，设置parseResult的错误标志
                    // 检查是否能继续解析请求体
                    if(readBuffer.readableBytes()<2) return false; // 数据不足，解析失败
                    std::vector<char> temp=readBuffer.lookDate(currLen,currLen+2);
                    if(temp[0]=='\r'&&temp[1]=='\n'){ // 出现换行，开始解析请求体
                        currLen+=2;
                        // 如果content-length不存在，则返回空字符串
                        std::string str=parseResult["content-length"];
                        if(!str.empty()){// str为空时，说明是GET请求，无需解析请求体，直接返回true
                            int bodyLen=bodyLen=std::stoi(str);
                            if(readBuffer.readableBytes()<bodyLen) return false; // 数据不足，解析失败
                            std::vector<char> body=readBuffer.lookDate(currLen,currLen+bodyLen);
                            currLen+=bodyLen;
                            readBuffer.abandonData(currLen); // 丢弃掉已经处理过的数据
                            parseResult["body"]=std::string(body.begin(),body.end());
                        }
                        return true; // 解析成功
                    }
                    break;
                }
                }
                currLine=""; // 解析完当前行后清空变量
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