# C++ MySQL库

`mysqllib.so`是C++操作MySQL数据库的函数库，该库提供了四个函数供外部程序调用：

update：根据数据库连接`MYSQK*`和SQL语句执行增删改操作，并返回操作结果，用于更新数据库。

query：根据数据库连接`MYSQL*`和SQL语句执行查询操作，如果查询成功，返回查询结果集。

getResult：取出查询结果集中的一条查询结果，将其封装成JSON字符串。

getAllResult：取出查询结果集中的所有查询结果，将其封装成JSON数组。

由于该C++ MySQL库是要提供给Python脚本调用的，因此需要特殊的导出方式。这里使用的是`boost`。

```shell
boost的安装命令：
sudo apt-get install libboost-all-dev
```

使用boost可以将C++编写的函数导出为Python可调用的模块，有以下几点需要注意：

* 需要将Python相关头文件所在目录加到程序的包含路径中（否则会报找不到某些头文件的错误）。
* 需要链接库文件 boost_python38（对于Python3.8而言） ，该文件在` /usr/lib/x86_64-linux-gnu`中可以找到。
* 对于生成的动态链接库（.so文件），需要与boost导出的模块同名，并且和Python脚本放在同一目录下，否则无法在Python中导入。

在Python中可以调用由boost导出的C++动态链接库，具体代码为：

```Python
# 动态链接库和boost导出的模块应该同名，作为Python导入时的模块名
import 模块名
# 调用导出的模块中的方法
模块名.方法名()
# 示例
# conn是从C++主程序中传递过来的MySQL连接
def query(conn):
    import mysqllib
    sql="insert into test values(1,2,3)"
    result=mysqllib.update(conn,sql)
    if result==True:
        print("执行成功")
    else:
        print("执行失败")
```

