#include "TimerManager.h"
#include <iostream>
using namespace std;
void TimerManager::addTimer(shared_ptr<Timer> timer,HttpConnect* con)
{
	unique_lock<mutex> locket(mtx);	
	if(con==nullptr)
	  return;
	shared_ptr<Timer> oldTimer=con->GetTimer();
	if(oldTimer!=nullptr)
	  oldTimer->SetInvalid();
	con->SetTimer(timer);
	timerQueue.push(timer);
}
int TimerManager::getNextExpireTime()
{
	unique_lock<mutex> locker(mtx);
	int res=-1;
	TimeStamp now=Timer::now();
	while(!timerQueue.empty())
	{
		shared_ptr<Timer> t=timerQueue.top();
		if(t->IsValid())
		{
			res=chrono::duration_cast<MilliSecond>(t->GetTime()-now).count();
			if(res>0)
			  return res;
			else
			{
				t->CallBack();
				timerQueue.pop();
			}
		}
		else
		{
		  timerQueue.pop();
		}
	}
	return -1;
}
