#include "ThreadPool.h"
using namespace std;

ThreadPool::ThreadPool(int  threadNum):stop(false),threads(threadNum)
{
	for (int i = 0; i < threadNum; i++)
		threads[i] = thread(Run,this);
}
ThreadPool::~ThreadPool()
{
	unique_lock<mutex> locker(mtx);
	stop = true;
	locker.unlock();
	cond.notify_all();
	for (size_t i = 0; i < threads.size(); i++)
		threads[i].join();
}

void ThreadPool::Run(ThreadPool *pool)
{
	while (true)
	{
		unique_lock<mutex> locker(pool->mtx);
		while (!pool->stop&&pool->tasks.empty())
			pool->cond.wait(locker);
		if (pool->stop&&pool->tasks.empty())
		{
			//complete
			return;
		}
		Task nextTask = pool->tasks.front();
		pool->tasks.pop();
		locker.unlock();
		nextTask();
	}
}

bool ThreadPool::PushJob(const Task& t)
{
	unique_lock<mutex> locker(mtx);
	if (!stop)
	{
		tasks.push(t);
		locker.unlock();
		cond.notify_one();
		return true;
	}
	return false;

}
