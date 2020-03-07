#ifndef _EPOLL_H_
#define _EPOLL_H_

#include <sys/epoll.h>
#include <vector>

#define MAXEVENTS 1024

class Epoll
{
public:
	Epoll();
	~Epoll();
	int add(int fd,void* ptr,int events);
	int mod(int fd,void* ptr,int events);
	int del(int fd,void* ptr,int events);
	int wait(int timeOutMs);
	epoll_event GetEvent(int i){return eventList[i];}
private:
	int epollFd;
	std::vector<struct epoll_event> eventList;
};

#endif
