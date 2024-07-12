#include <iostream>
#include <thread>

#include "ThreadPool.h"

namespace threadpool
{
	ThreadPool::ThreadPool(int min, int max)
	{
		do
		{
			//实例化当前线程池的唯一任务队列
			m_taskQueue = new taskqueue::TaskQueue;

			//初始化线程池中的线程管理参数
			m_minThreads = min;
			m_maxThreads = max;
			m_busyThreads = 0;
			m_exitThreads = 0;
			m_aliveThreads = m_minThreads;
			m_shutdown = false;

			//初始化线程组
			m_threadIds.resize(m_minThreads);
			if (m_threadIds.empty())
			{
				std::cerr << "vector thread failed!" << std::endl;
				break;
			}

			for (int i = 0; i < m_minThreads; ++i)
			{
				//emplace_back避免额外的复制消耗
				m_threadIds.emplace_back(new std::thread(&ThreadPool::worker, this));
				std::cout << "create thread ID: " << m_threadIds.back()->get_id() << std::endl;
			}

			m_mangerId = std::thread(manger);
			std::cout << "create Mangerthread ID: " << m_mangerId.get_id() << std::endl;
		}
		while (false);
	}

	ThreadPool::~ThreadPool()
	{
		m_shutdown = true;

		if (m_mangerId.joinable())
		{
			std::cout << "Manager thread ID: " << m_mangerId.get_id() << " is exiting" << std::endl;
			m_mangerId.join();
		}

		// 等待工作线程退出
		for (int i = 0; i < m_aliveThreads; ++i)
		{
			if (m_threadIds[i]->joinable())
			{
				std::cout << "Thread ID " << m_threadIds[i]->get_id() << " is exiting" << std::endl;
				m_threadIds[i]->join();
			}
			delete m_threadIds[i];
		}


		// 释放任务队列内存
		delete m_taskQueue;
		m_taskQueue = nullptr;
	}

	inline void ThreadPool::add_task(taskqueue::Task task)
	{
		m_taskQueue->addTask(task);
		cond.notify_all();
	}

	inline size_t ThreadPool::getBusyThreadNum()
	{
		std::lock_guard locker(m_mutex);
		return m_busyThreads;
	}

	inline size_t ThreadPool::getAliveThreadNum()
	{
		std::lock_guard locker(m_mutex);
		return m_busyThreads;
	}

	void ThreadPool::threadExit()
	{
		auto tid = std::this_thread::get_id();

		auto iter = std::find_if(m_threadIds.begin(), m_threadIds.end(),
		                         [&](std::thread* t) { return t->get_id() == tid; });

		if (iter != m_threadIds.end())
		{
			delete*iter;
			*iter = nullptr;
		}
	}

	void ThreadPool::worker(void* arg)
	{
		auto pool = static_cast<ThreadPool*>(arg);

		while (true)
		{
			std::unique_lock<std::mutex> locker(pool->m_mutex);

			//1.当任务队列为空且线程池未关闭时,阻塞工作线程
			while (pool->m_taskQueue->getTaskCount() == 0 && !pool->m_shutdown)
			{
				std::cout << "thread " << std::this_thread::get_id() << " is waiting..." << std::endl;

				pool->cond.wait(locker);

				//接触阻塞后判断是否需要销毁线程
				if (pool->m_exitThreads > 0)
				{
					pool->m_exitThreads--;
					if (pool->m_aliveThreads > pool->m_minThreads)
					{
						pool->m_aliveThreads--;
						std::cout << "Manger kill thread ID: " << std::this_thread::get_id() << std::endl;
						locker.unlock();
						pool->threadExit();
						return;
					}
				}
			}

			// 2. 线程池关闭：退出当前工作线程
			if (pool->m_shutdown)
			{
				std::cout << "Thread " << std::this_thread::get_id() << " is shutting down." << std::endl;
				return;
			}

			// 3. 从队列中取出任务并执行
			auto task = pool->m_taskQueue->takeTask();
			pool->m_busyThreads++;
			locker.unlock();

			//执行任务
			std::cout << "thread " << std::this_thread::get_id() << " is working." << std::endl;
			task();

			//任务完成,更新数量
			locker.lock();
			std::cout << "Thread " << std::this_thread::get_id() << " has finished." << std::endl;
			pool->m_busyThreads--;
			locker.unlock();
		}
	}

	void ThreadPool::manger()
	{
	}
}
