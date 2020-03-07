#ifndef _MYWEBSERVER_H_
#define _MYWEBSERVER_H_

#include <memory>

#include "Epoll.h"
#include "HttpConnect.h"
#include "ThreadPool.h"
#include "TimerManager.h"

const int CONNECT_TIMEOUT=500;

class MyWebServer
{
public:

	MyWebServer(int port_=4000,int backlog_=10,int timeout_=500,int threadNum_=1000,std::string root_="");
	//服务器运行
	void run();
	
	~MyWebServer();

private:
	//初始化服务器socket
	bool InitSocket();
	//用于接收数据
	void HttpRecv(HttpConnect* con);
	//用于发送数据
	void HttpSend(HttpConnect* con);
	//设置非阻塞socket
	void SetNonBlock(int fd);
	//建立连接
	void AcceptConnection();
	//关闭连接
	void CloseConnection(HttpConnect* fd);

private:
	//服务器端口号
	int port;
	//listen最大值
	int backlog;
	//连接超时时间
	int timeout;
	//线程数
	int threadNum;
	//文件存放的根路径
	std::string root;
	//服务器socket
	int socket_fd;
	//服务器连接类，服务器无需与自己交流数据,该变量仅为了记录fd,顺便关闭socket
	std::unique_ptr<HttpConnect> serverCon;
	//epoll
	std::unique_ptr<Epoll> epoll;
	//线程池
	std::unique_ptr<ThreadPool> threadPool;
	//计时器
	std::unique_ptr<TimerManager> timerManager;
};

#endif
