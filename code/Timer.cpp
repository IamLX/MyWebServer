#include "Timer.h"
Timer::Timer(int timeout,const std::function<void()>& fun):
	outTime(now()+static_cast<MilliSecond>(timeout)),
	valid(true),
	TimeOutCallBack(fun)
{

}
