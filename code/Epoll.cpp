#include "Epoll.h"
#include <iostream>
#include <unistd.h> //close
using namespace std;

Epoll::Epoll():epollFd(epoll_create(MAXEVENTS)),eventList(MAXEVENTS)
{
}

Epoll::~Epoll()
{
	close(epollFd);
}

int Epoll::add(int fd,void* ptr,int events)
{
	struct epoll_event event;
	event.data.ptr=ptr;
	event.events=events;
	return epoll_ctl(epollFd,EPOLL_CTL_ADD,fd,&event);
}

int Epoll::mod(int fd,void* ptr,int events)
{
	struct epoll_event event;
	event.data.ptr=ptr;
	event.events=events;
	return epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&event);

}

int Epoll::del(int fd,void* ptr,int events)
{
	struct epoll_event event;
	event.data.ptr=ptr;
	event.events=events;
	return epoll_ctl(epollFd,EPOLL_CTL_DEL,fd,&event);
}

int Epoll::wait(int timeOutMs)
{
	int eventsNum=epoll_wait(epollFd,eventList.data(),static_cast<int>(eventList.size()),timeOutMs);	
	if(eventsNum<0)
	  cout<<"[Epoll::wait(): events error]"<<endl;
	return eventsNum;
}



