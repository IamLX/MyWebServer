# MyWebServer
## 项目介绍
该项目为C++11实现的简单的web服务器，能够并发处理请求，使用reactor模型，读写均为ET触发，包含了epoll、线程池、定时器。
写该项目的目的是为了学习web服务器的工作流程和实现并发，目前只处理了Get请求。

##编译运行环境
操作系统:ubuntu_16_04_x64
编译器:g++ 5.4.0

##运行方法
cd MyWebServer/code
make
./Server [-p port] [-b backlog] [-o timeout] [-t threadNum] [-r filePath]


