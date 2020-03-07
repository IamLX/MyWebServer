# MyWebServer

## 简介
该项目为C++11实现的简单的web服务器，使用reactor模型，读写均为ET触发，包含了epoll、线程池、定时器，可以处理一定规模的并发。

## 开发工具
* 操作系统:ubuntu_16_04_x64
* 编译器:g++ 5.4.0

## 运行方法
cd MyWebServer/code
make
./Server  [-p   port]   [-b   backlog]   [-o   timeout]   [-t   threadNum]   [-r   filePath]

## 代码统计



![codePic](codePic.png)



## 其它

|         项目目的          |         设计结构          |         并发测试          |
| :-----------------------: | :-----------------------: | :-----------------------: |
| [项目目的](./项目目的.md) | [设计结构](./设计架构.md) | [并发测试](./并发测试.md) |







