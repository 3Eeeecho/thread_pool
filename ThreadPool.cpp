#include "ThreadPool.h"

ThreadPool::ThreadPool(int min, int max)
{

}

ThreadPool::~ThreadPool()
{

}

void ThreadPool::addTask(Task task)
{
	m_taskQueue->addTask(task);

	cond.notify_all();
}

size_t ThreadPool::getBusyThreadNum()
{
	size_t busyNum = 0;
	std::lock_guard<std::mutex> locker(m_mutex);
	busyNum = m_busyThreads;
	return busyNum;
}

size_t ThreadPool::getAliveThreadNum()
{
	size_t aliveNum = 0;
	std::lock_guard<std::mutex> locker(m_mutex);
	aliveNum = m_busyThreads;
	return aliveNum;
}
