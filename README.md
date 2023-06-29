# Ray——后端服务器框架
## 项目介绍

本项目是一个基于`C++`, `Python`, `Linux`, `MySQL`的后端服务器框架。该项目已经开发好的部分包括：核心服务器（基于C++）、内置的Python解释器、MySQL函数库（基于C++）、Python接口层（与C++代码进行对接，方便Python业务层快速开发）。用户只需要使用Python脚本编写业务层逻辑，即可快速搭建一个简易的后端服务器。

核心服务器主要实现的功能有：

* 异步的日志系统，方便在开发中进行调试以及在服务器运行时监控服务器的运行状态。
* 单例模式的线程池，避免频繁创建和销毁线程带来的额外开销。
* 单例模式的数据库（MySQL）连接池，避免频繁创建和销毁连接带来的额外开销。
* 基于小根堆的定时器，用于清除到期的非活跃的连接。
* 自定义的可以自动增长的缓冲区，用于读取文件中的数据或者向文件中写数据。
* 封装epoll函数，使用I/O多路复用监听文件描述符的状态。
* Python脚本引擎，用于执行Python脚本，并获取Python函数的返回值。
* 利用正则表达式和状态机解析HTTP请求报文，并路由给相应的处理函数。（待开发）
* 根据处理结果和实际情况封装HTTP响应报文，返回给客户端。（待开发）
* 使用基于特定规则的报文过滤器，实现防火墙等功能。（待开发）
* 使用基于特定规则的报文转发器，实现反向代理、动静分离、负载均衡等功能。（待开发）

Python脚本：

* Python内层接口模块对C++ MySQL函数库进行进一步的封装，提供给业务层调用。
* Python外层接口模块要根据HTTP请求把数据路由给相应的处理函数。（待开发）
* 搭建一个合理的Python业务层框架。（待开发）

## 项目结构

项目结构主要分为三层：

* 外层：服务器层。数据报经过过滤、转发后，传递给业务层进行处理。
* 中间层：业务层。调用数据库层的功能，处理传递过来的数据，并将结果封装后返回给服务器层。
* 内层：数据库层。给业务层提供一些可调用的功能。

在程序运行过程中，服务器层将接收到的数据初步解析后传递给业务层，业务层对数据进行处理，并调用数据库层的功能，得到处理结果。业务层将处理结果封装后返回给服务器层，由服务器层将数据发送给客户端。外层依次调用内层的接口，内层将数据依次返回给外层。

```Python
-C++核心服务器
	--HTTP解析器（解析HTTP请求）
		---Python外层接口（路由到相应的函数）
			----Python业务层
				-----Python内层接口（封装C++ MySQL函数库）
	--HTTP封装器（封装HTTP响应）		
```

## 目录结构

```python
-boost				# boost.python开发环境（使用boost.python所需文件）
 --include			# boost.python相关头文件
 --lib				# boost.python相关库文件
-MySQL				# MySQL开发环境（连接MySQL所需文件）
 --inlude			# MySQL开发相关头文件
 --lib				# MySQL开发相关库文件
-Python				# Python开发环境（嵌入Python解释器所需文件）
 --include			# Python开发相关头文件
 --lib				# Python开发相关库文件
-script				# 预定义的Python模块
 --pmysql			# 对C++ MySQL函数库进一步封装
-server				# 
 --Buffer
 --Epoll
 --Log
 --MySQLPool
 --RunPython
 --Server
 --ThreadPool
 --Timer
...未完待续
 --config.ini		# 服务器启动时要读取的配置文件
 --CMakeLists.txt	# CMake
 --README.md		# 概要说明文件
-sql				# C++编写的MySQL函数库
 --CMakeLists.txt	# CMake
 --README.md		# 概要说明文件
 --mysqllib.cpp		# C++编写的MySQL库（源文件）
 --mysqllib.so		# C++编写的MySQL库（动态库，由boost导出，可用Python调用）
-ui					#
-.gitignore			# git忽略
-LICENSE			# Apache2.0 开源许可
-README.md			# README.md文件，Ray的概要说明文件
```

## 压力测试

测试程序：·webbench·

测试环境: `Ubuntu20.04` `cpu` `内存:4G`

测试结果：`QPS 10000+`

## 项目编译

## 项目启动

编写好相应的配置文件，创建好项目脚本，使用`ui`启动项目

## 项目发布

## 致谢
