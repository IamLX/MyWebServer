#ifndef _TIMER_H_
#define _TIMER_H_
#include <chrono>
#include <functional>

class Timer
{
public:
	using TimeStamp = std::chrono::high_resolution_clock::time_point;
	using Clock=std::chrono::high_resolution_clock;
	using MilliSecond=std::chrono::milliseconds;
	
	static TimeStamp now(){return Clock::now();};
	Timer(int timeout,const std::function<void()>& fun);
	bool operator< (const Timer&t)const {return outTime<t.outTime;}
	bool IsValid(){return valid;}
	void SetInvalid(){valid=false;}
	void CallBack(){TimeOutCallBack();}
	TimeStamp GetTime(){return outTime;}
private:
	TimeStamp outTime;
	bool valid;
	std::function<void()> TimeOutCallBack;
};

#endif
