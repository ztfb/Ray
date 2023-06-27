该文件夹下是C++调用python代码所必须的头文件和库文件，其相当于一个简化版的python解释器。以下是构建内置python解释器的流程：

1.去python官网：https://www.python.org 下载Python-3.8.6.tgz
2.使用命令tar -zxvf Python-3.8.6.tgz 打开压缩包
3.安装编译python揭示其所需依赖：apt-get install zlib-devel bzip2-devel openssl-devel ncurses-devel sqlite-devel readline-devel tk-devel gcc make
4.进入python解释器文件夹中，执行：./configure --prefix=/usr/local/python38 --enable-shared  --enable-optimizations --enable-static
--enable-optimizations 是优化选项(LTO，PGO 等)加上这个 flag 编译后，性能有 10% 左右的优化
--enable-static：生成静态链接库
--enable-shared：生成动态链接库
5.进行编译：make
6.进行安装：sudo make altinstall
7.将安装好的python解释器中的include目录取出
8.将python解释器中的lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8.a取出，将python解释器lib目录下的三个动态链接库取出
