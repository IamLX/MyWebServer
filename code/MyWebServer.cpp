#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <errno.h>

#include "MyWebServer.h"
using namespace std;

MyWebServer::MyWebServer(int port_,int backlog_,int timeout_,int threadNum_,string root_):
	port(port_),
	backlog(backlog_),
	timeout(timeout_),
	threadNum(threadNum_),
	root(root_),
	epoll(new Epoll()),
	threadPool(new ThreadPool(threadNum_)),
	timerManager(new TimerManager())
{
	//初始化服务器socket
	if(!InitSocket())
	{
		cout<<"[MyWebServer::run()]: socket init failed"<<endl;
		return;
	}
	serverCon.reset(new HttpConnect(socket_fd));
}

//socket设为非阻塞
void MyWebServer::SetNonBlock(int fd)
{
	int old_option=fcntl(fd,F_GETFL);
	int new_option=old_option|O_NONBLOCK;
	fcntl(fd,F_SETFL,new_option);
}

//初始化服务器socket
bool MyWebServer::InitSocket()
{
	struct sockaddr_in addr;
	socket_fd=socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd==-1)
	{
		cout<<"[MyWebServer::initSocket]  create socket failed"<<endl;
		return false;
	}
	
	//端口复用
	int opt=1;
	setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
	//设置地址端口
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);
	//设置非阻塞
	SetNonBlock(socket_fd);
	if(bind(socket_fd,(struct sockaddr*)&addr,sizeof(addr))==-1)
	{
		cout<<"[MyWebServer::initSocket:] bind failed"<<endl;
		return false;
	}

	if(listen(socket_fd,backlog)==-1)
	{
		cout<<"[MyWebServer::initSocket:] listen failed"<<endl;
		return false;
	}
	return true;	
}

void MyWebServer::run()
{
//将服务器socket加入epoll
	epoll->add(socket_fd,static_cast<void*>(serverCon.get()),(EPOLLIN|EPOLLET));
	
	while(true)
	{
		int waitTime=timerManager->getNextExpireTime();
		int eventsNum=epoll->wait(waitTime);
		if(eventsNum>0)
		{
			for(int i=0;i<eventsNum;i++)
			{
				epoll_event event=epoll->GetEvent(i);
				HttpConnect* eventCon=static_cast<HttpConnect*>(event.data.ptr);
				//有新连接
				if(eventCon->GetFd()==socket_fd)
				{
					AcceptConnection();
				}
				else if( event.events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ))
				{
					//出错，关闭fd的连接
					eventCon->GetTimer()->SetInvalid();
					eventCon->SetWorking(false);
					CloseConnection(eventCon);
				}
				else if(event.events&EPOLLIN)
				{
					eventCon->SetWorking(true);
					threadPool->PushJob(bind(&MyWebServer::HttpRecv,this,eventCon));
				}
				else if(event.events&EPOLLOUT)
				{
					eventCon->SetWorking(true);
					threadPool->PushJob(bind(&MyWebServer::HttpSend,this,eventCon));
				}
			}
		}
	}
}
//接收连接
void MyWebServer::AcceptConnection()
{
	while(true)
	{
		struct sockaddr_in client;
		socklen_t client_addrlen=sizeof(client);
		int connect_fd=accept(socket_fd,(struct sockaddr*)&client,&client_addrlen);
		if(connect_fd<0)
		{
			if(errno!=EAGAIN)
				cout<<"[MyWebServer::run()]: accept failed"<<endl;
			break;
		}
		SetNonBlock(connect_fd);
		static long long n=0;
		n++;
		HttpConnect* con=new HttpConnect(connect_fd,timeout,root);
		shared_ptr<Timer> timer(new Timer(timeout,bind(&MyWebServer::CloseConnection,this,con)));
		timerManager->addTimer(timer,con);
		epoll->add(connect_fd,static_cast<void*>(con),(EPOLLET|EPOLLIN|EPOLLONESHOT));
	}
}

//关闭连接
void MyWebServer::CloseConnection(HttpConnect* con)
{
	if(con->IsWorking())
	  return;
	epoll->del(con->GetFd(),static_cast<void*>(con),0);
	
	if(con->GetTimer()!=nullptr)
		con->GetTimer()->SetInvalid();  
	static long long d=0;
	d++;
	delete con;
	con=nullptr;
}
//接收数据
void MyWebServer::HttpRecv(HttpConnect *con)
{
	con->GetTimer()->SetInvalid();
	//接收数据
	HttpConnect::RetCode ret=con->ReadSock();

	if(ret==HttpConnect::NotConnected)
	{
		con->SetWorking(false);
		//连接已断开
		CloseConnection(con);
		return;
	}
	else if(ret==HttpConnect::Fail)
	{
		//没收到消息，但仍连接
		con->SetWorking(false);
		epoll->mod(con->GetFd(),static_cast<void*>(con),EPOLLIN|EPOLLET|EPOLLONESHOT);
		return;
	}
	
	if(con->ProcessRequest())
	{
		//报文解析完成，添加回送报文
		con->AddResponse(200);
		epoll->mod(con->GetFd(),static_cast<void*>(con),EPOLLET|EPOLLOUT | EPOLLONESHOT);
	}
	else
	{
		//报文解析失败
		con->AddResponse(400);
		con->SetWorking(false);
		CloseConnection(con);
		return;
	}
}
//发送数据
void MyWebServer::HttpSend(HttpConnect* con)
{
	con->GetTimer()->SetInvalid();
	HttpConnect::RetCode ret=con->WriteSock();
	//非EAGAIN
	if(ret==HttpConnect::Fail)
	{
		con->SetWorking(false);
		CloseConnection(con);
		return;
	}
	else if(ret==HttpConnect::Success)
	{
	    		//发完之后，如果不是keep-alive，则断开
		if(!con->KeepAilive())
		{
			con->SetWorking(false);
			CloseConnection(con);
			return;
		}
		//如果发完之后没数据，则不可写
		shared_ptr<Timer> timer(new Timer(timeout,bind(&MyWebServer::CloseConnection,this,con)));
		timerManager->addTimer(timer,con);		
		if(con->NumToSend()>0)
			epoll->mod(con->GetFd(),static_cast<void*>(con),EPOLLET|EPOLLIN|EPOLLOUT|EPOLLONESHOT);
		else
			epoll->mod(con->GetFd(),static_cast<void*>(con),EPOLLET|EPOLLIN|EPOLLONESHOT);
		con->Reset();
	}

}



MyWebServer::~MyWebServer()
{
}
