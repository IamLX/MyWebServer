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



![codePic]( https://github.com/WFan99/MyWebServer/blob/master/doc/codePic.png )



## 其它

|         项目目的          |         设计结构          |         并发测试          |
| :-----------------------: | :-----------------------: | :-----------------------: |
| [项目目的](https://github.com/WFan99/MyWebServer/blob/master/doc/%E9%A1%B9%E7%9B%AE%E7%9B%AE%E7%9A%84.md) | [设计结构]( https://github.com/WFan99/MyWebServer/blob/master/doc/%E8%AE%BE%E8%AE%A1%E7%BB%93%E6%9E%84.md) | [并发测试]( https://github.com/WFan99/MyWebServer/blob/master/doc/%E5%B9%B6%E5%8F%91%E6%B5%8B%E8%AF%95.md ) |

