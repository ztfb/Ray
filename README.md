# Ray——后端服务器框架
## 项目介绍

本项目是一个基于`C++`, `Python`, `Linux`, `MySQL`的后端服务器框架。该项目已经开发好的部分包括：核心服务器（基于C++）、MySQL函数库（基于C++）、Python接口层（与C++代码进行对接，方便Python业务层快速开发）。用户只需要使用Python脚本编写业务层逻辑，即可快速搭建一个简易的后端服务器。

核心服务器主要实现的功能有：

* 异步的日志系统，方便在开发中进行调试以及在服务器运行时监控服务器的运行状态。
* 单例模式的线程池，避免频繁创建和销毁线程带来的额外开销。
* 单例模式的数据库（MySQL）连接池，避免频繁创建和销毁连接带来的额外开销。
* 基于小根堆的定时器，用于清除到期的非活跃的连接。
* 自定义的可以自动增长的缓冲区，用于读取文件中的数据或者向文件中写数据。
* 封装epoll函数，使用I/O多路复用监听文件描述符的状态。
* Python脚本引擎，用于执行Python脚本，并获取Python函数的返回值。
* 利用正则表达式和状态机解析HTTP请求报文，并路由给相应的处理函数。
* 根据处理结果和实际情况封装HTTP响应报文，返回给客户端。
* 使用基于特定规则的报文过滤器，可以实现防火墙等功能。
* 使用基于特定规则的报文转发器，可以实现反向代理等功能。

MySQL函数库：

* 对MySQL的增删改查进行了简单的封装，使用户调用MySQL更加方便。MySQL的查询结果统一封装成`json`字符串返回。

Python脚本：

* Python内层接口模块（`pmysql`）对C++ MySQL函数库进行进一步的封装，实现了简单的增删改语句的自动拼接，提供给业务层调用。
* Python外层接口模块（`prouter`）会根据HTTP请求的请求方法和URL将请求路由给相应的函数处理。
* 制定了一个Python业务层开发规范。

程序的可扩展性：

* 该后端框架可以很容易的更换所操作的数据库，实现和其他数据库的连接（只要按照MySQL函数库的规范重新开发一套C++函数库和Python内层接口模块即可）。
* 该后端框架可以很容易的更换上层协议，只要重写Connection的process方法调用的process方法，以及Python外层接口模块即可（即把HTTP处理器和`prouter`模块更换成其他协议的处理器和处理模块即可）。

## 项目结构

项目结构主要分为三层：

* 外层：服务器层。数据报经过过滤、转发和简单的解析后，传递给业务层进行处理。
* 中间层：业务层。调用数据库层的功能，处理传递过来的数据，并将结果封装后返回给服务器层。
* 内层：数据库层。给业务层提供一些可调用的功能。

在程序运行过程中，服务器层将接收到的数据初步解析后传递给业务层，业务层对数据进行处理，并调用数据库层的功能，得到处理结果。业务层将处理结果封装后返回给服务器层，由服务器层将数据进一步封装后发送给客户端。外层依次调用内层的接口，内层将数据依次返回给外层。

```Python
-C++核心服务器
	--HTTP处理器
		---Python外层接口
			----Python业务层
				-----Python内层接口
                	------C++ MySQL函数库
# 从上到下是依次调用的关系
# 数据则从下到上依次返回	
```

## 目录结构

```python
# 将boost MySQL Python的相关头文件和库文件放在项目中，是为了方便在没有安装过这些环境的PC上进行编译
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
 --prouter			# 将HTTP请求路由给相应的函数，并将处理结果返回给HTTP处理器
 --pscript			# 用户编写的业务层代码
-server				# C++核心服务器
 --Buffer			# 自动增长的缓冲区
 --Epoll			# epoll相关函数封装
 --Log				# 异步日志系统
 --MySQLPool		# MySQL连接池
 --RunPython		# Python脚本引擎
 --Server			# 通信主循环
 --ThreadPool		# 线程池
 --Timer			# 基于小根堆的定时器
 --Connection		# 客户端连接封装
 --HttpProcess		# HTTP处理器
 --main.cpp			# 主程序（启动程序）
 --test				# 一个简单的客户端代码，可以用于测试服务器
 --config.ini		# 服务器启动时要读取的配置文件
 --CMakeLists.txt	# CMake
 --README.md		# 概要说明文件
-sql				# C++编写的MySQL函数库
 --CMakeLists.txt	# CMake
 --README.md		# 概要说明文件
 --mysqllib.cpp		# C++编写的MySQL库（源文件）
 --mysqllib.so		# C++编写的MySQL库（动态库，由boost导出，可用Python调用）
-.gitignore			# git忽略
-LICENSE			# Apache2.0 开源许可
-README.md			# README.md文件，Ray的概要说明文件
```

## 压力测试

测试程序：`webbench`

由于不同的环境下测试结果相差很大，因此这里不再做统一的测试，用户可自行测试。

## 项目编译、启动与发布

### 项目编译

* `server`的编译：使用源代码目录下的`CMakeLists.txt`文件进行编译，编译后生成一个可执行文件`server`。
* `mysqllib`的编译：使用源代码目录下的`CMakeLists.txt`文件进行编译，编译后生成一个动态链接库`mysqllib`。

注意：由于路径的差异或环境的差异，用户可能需要对`CMakeLists.txt`文件进行一定的修改才能重新编译！！

### 项目启动

将编译后生成的可执行文件`server`，服务器配置文件`config.ini`，C++ MySQL函数库文件`mysqllib.so`放在同一个文件夹下。在同级目录下创建`script`文件夹，将`pmysql`、`prouter`、`pscript`模块都放到该文件夹下。然后在终端中输入`./server`即可启动服务器。

```python
# 如果配置文件与server不在同一目录下，或Python脚本不在server目录的script文件夹下
# 终端中启动server时要携带参数：
./server 配置文件路径 脚本文件所在文件夹（两者顺序不能乱）
```

### 项目发布

将编写好的业务层Python代码直接放到Ray的script文件夹下，然后将整个项目（包括`server`、`mysqllib.so`、`config.ini`、`script`）直接压缩打包即可发布。得到发布压缩包后，先打开压缩包，然后使用上述方法即可启动项目。

## 致谢

@ `LZF`

