#include "ThreadPool.h"

#include <iostream>
#include <threads.h>

ThreadPool::ThreadPool(int min, int max)
{
	do
	{
		//实例化当前线程池的唯一任务队列
		m_taskQueue = new TaskQueue;

		//初始化线程池中的线程管理参数
		m_minThreads = min;
		m_maxThreads = max;
		m_busyThreads = 0;
		m_aliveThreads = m_minThreads;
		m_shutdown = false;

		//初始化线程组
		m_threadIds.resize(m_minThreads);
		if(m_threadIds.empty())
		{
			std::cerr << "vector thread failed!" << std::endl;
			break;
		}

		for(int i=0;i<m_minThreads;++i)
		{
			m_threadIds[i] = new std::thread(worker);
			std::cout << "create thread ID: " << m_threadIds[i]->get_id() << std::endl;
		}

		m_mangerId = std::thread(manger);
		std::cout << "create Mangerthread ID: " << m_mangerId.get_id() << std::endl;

	}
	while (0);

}

ThreadPool::~ThreadPool()
{
	m_shutdown = true;

	if(m_mangerId.joinable())
	{
		std::cout << "Manager thread ID: " << m_mangerId.get_id() << " is exiting" << std::endl;
		m_mangerId.join();
	}

	// 等待工作线程退出
	for (int i = 0; i < m_aliveThreads; ++i) {
		if (m_threadIds[i]->joinable()) {
			std::cout << "Thread ID " << m_threadIds[i]->get_id() << " is exiting" << std::endl;
			m_threadIds[i]->join();
		}
		delete m_threadIds[i];
	}


	// 释放任务队列内存
	delete m_taskQueue;
	m_taskQueue = nullptr;

}

void ThreadPool::addTask(Task task)
{
	m_taskQueue->addTask(task);
	cond.notify_all();
}

size_t ThreadPool::getBusyThreadNum()
{
	std::lock_guard<std::mutex> locker(m_mutex);
	return m_busyThreads;
}

size_t ThreadPool::getAliveThreadNum()
{
	std::lock_guard<std::mutex> locker(m_mutex);
	return m_busyThreads;
}

void ThreadPool::threadExit()
{
	auto tid = std::this_thread::get_id();

	auto iter = std::find_if(m_threadIds.begin(), m_threadIds.end(), [&](std::thread* t) {return t->get_id() == tid; });

	if(iter!=m_threadIds.end())
	{
		delete *iter;
		*iter = nullptr;
	}

}

void ThreadPool::worker()
{

}

void ThreadPool::manger()
{

}


