#include "MySQLPool.h"
#include "Log.h"

static std::shared_ptr<MySQLPool> mysqlPool=nullptr;
static std::mutex mutex;

std::shared_ptr<MySQLPool> MySQLPool::instance(){
    // 懒汉模式
    // 使用双重检查保证线程安全
    if(mysqlPool==nullptr){
        std::unique_lock<std::mutex> lock(mutex); // 访问临界区之前需要加锁
        if(mysqlPool==nullptr){
            mysqlPool=std::shared_ptr<MySQLPool>(new MySQLPool());
        }
    }
    return mysqlPool;
}

void MySQLPool::init(const std::string& host,int port,const std::string& username,const std::string& password,
                const std::string& dbname,int connNum)
{
    for(int i=0;i<connNum;i++){
        MYSQL* mysql=nullptr;
        mysql=mysql_init(mysql);
        if(!mysql){
            log_error("MySQL初始化失败...");
            return ;
        }
        mysql=mysql_real_connect(mysql,host.c_str(),username.c_str(),password.c_str(),dbname.c_str(),port,NULL,0);
        if(!mysql){
            log_warn("MySQL连接失败...");
            return ;
        }
        log_info("MySQL连接成功...");
        connQue.push(mysql); // 将成功的连接加到连接队列中
    }
}

void MySQLPool::close(){
    std::unique_lock<std::mutex> lock(poolLock);
    while(!connQue.empty()) {
        // 将创建好的连接从队列中取出并关闭
        auto mysql = connQue.front();
        connQue.pop();
        mysql_close(mysql);
    }
    mysql_library_end();  
}

MYSQL* MySQLPool::getConn(){
    // 从连接队列中取出一个连接使用
    MYSQL *mysql = nullptr;
    if(connQue.empty()){
        log_warn("MySQL连接池繁忙...");
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(poolLock);
    mysql=connQue.front();
    connQue.pop();
    return mysql;
}
void MySQLPool::freeConn(MYSQL *mysql){
    // 将一个用完的连接重新添加到队列中
    std::unique_lock<std::mutex> lock(poolLock);
    connQue.push(mysql); // 将连接重新加入连接队列
}