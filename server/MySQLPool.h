#ifndef MYSQLPOOL
#define MYSQLPOOL

#include <memory>
#include <mutex>
#include <string>
#include <queue>
#include <mysql/mysql.h>

class MySQLPool{
public:
    static std::shared_ptr<MySQLPool> instance(); // 获取MySQLPool的单例对象
    bool init(const std::string& host,int port,const std::string& username,const std::string& password,
                const std::string& dbname,int connNum);
    
    void close(); // 关闭连接池
    MYSQL* getConn(); // 获取一个连接
    void freeConn(MYSQL *mysql); // 释放一个连接
    
    MySQLPool(const MySQLPool&) = delete; // 禁用拷贝构造函数
    MySQLPool& operator=(const MySQLPool&) = delete; // 禁用赋值运算符
    ~MySQLPool();
private:
    MySQLPool() = default; // 禁用外部构造
    std::mutex poolLock; // 连接池互斥锁
    std::queue<MYSQL*> connQue; // 连接队列
};

#endif