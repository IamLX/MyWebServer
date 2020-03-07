#ifndef _TIMERMANAGER_H_
#define _TIMERMANAGER_H_


#include "Timer.h"
#include "HttpConnect.h"
#include <queue>
#include <mutex>
#include <memory>
class TimerManager
{
public:
	using TimeStamp=Timer::TimeStamp;
	using Clock=Timer::Clock;
	using MilliSecond=Timer::MilliSecond;

	TimerManager()=default;
	void addTimer(std::shared_ptr<Timer> timer,HttpConnect* con);
	int getNextExpireTime();

private:
	struct cmp
	{
		bool operator()(std::shared_ptr<Timer> a, std::shared_ptr<Timer>b)
		{
			return *b<*a;
		}
	};

	std::priority_queue<std::shared_ptr<Timer>,std::vector<std::shared_ptr<Timer>>,cmp> timerQueue;
	std::mutex mtx;
};

#endif
