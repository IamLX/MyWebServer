#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <vector>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
class ThreadPool
{
public:
	using Task=std::function<void()>;
	ThreadPool(int);
	bool PushJob(const Task&);
	~ThreadPool();
private:
	static void Run(ThreadPool*);
private:
	bool stop;
	std::vector<std::thread> threads;
	std::mutex mtx;
	std::condition_variable cond;
	std::queue<Task> tasks;
};

#endif
