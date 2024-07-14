#include <iostream>
#include <thread>

#include "ThreadPool.h"

namespace threadpool
{
	ThreadPool::ThreadPool(int min, int max)
		: m_minThreads(min), m_maxThreads(max), m_busyThreads(0), m_aliveThreads(m_minThreads),
		  m_exitThreads(0), m_shutdown(false)
	{
		try
		{
			// 初始化任务队列
			m_taskQueue = std::make_unique<taskqueue::TaskQueue>();

			// 初始化线程组

			for (int i = 0; i < m_minThreads; ++i)
			{
				m_threadIds.emplace_back(new std::thread(&ThreadPool::worker, this));
				{
					std::lock_guard locker(m_outputMutex);
					std::cout << "Created thread: " << m_threadIds[i]->get_id() << std::endl;
				}
			}

			m_mangerId = std::thread(&ThreadPool::manger, this);
			{
				std::lock_guard locker(m_outputMutex);
				std::cout << "Created Manager thread: " << m_mangerId.get_id() << std::endl;
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Exception in ThreadPool constructor: " << e.what() << std::endl;
			// 如果发生异常，确保清理工作
			m_shutdown = true;
			if (m_mangerId.joinable())
				m_mangerId.join();
			for (int i = 0; i < m_threadIds.size(); ++i)
			{
				if (m_threadIds[i] && m_threadIds[i]->joinable())
					m_threadIds[i]->join();
				delete m_threadIds[i];
			}
			throw; // 继续抛出异常
		}
	}

	ThreadPool::~ThreadPool()
	{
		m_shutdown = true;

		if (m_mangerId.joinable())
		{
			{
				std::lock_guard locker(m_outputMutex);
				std::cout << "Manager thread: " << m_mangerId.get_id() << " is exiting" << std::endl;
			}
			m_mangerId.join();
		}

		cond.notify_all();
		// 等待工作线程退出
		for (int i = 0; i < m_aliveThreads; ++i)
		{
			if (m_threadIds[i] && m_threadIds[i]->joinable())
			{
				{
					std::lock_guard locker(m_outputMutex);
					std::cout << "Thread " << m_threadIds[i]->get_id() << " is exiting" << std::endl;
				}
				m_threadIds[i]->join();
			}
			delete m_threadIds[i];
		}
	}

	void ThreadPool::addTask(taskqueue::Task task)
	{
		{
			std::lock_guard locker(m_mutex);
			m_taskQueue->addTask(task);
		}
		cond.notify_one();
	}

	size_t ThreadPool::getBusyThreadNum()
	{
		std::lock_guard locker(m_mutex);
		return m_busyThreads;
	}

	size_t ThreadPool::getAliveThreadNum()
	{
		std::lock_guard locker(m_mutex);
		return m_aliveThreads;
	}

	void ThreadPool::threadExit()
	{
		auto tid = std::this_thread::get_id();
		std::lock_guard locker(m_mutex); // 加锁保护操作
		auto iter = std::find_if(m_threadIds.begin(), m_threadIds.end(),
		                         [&](std::thread* t) { return t->get_id() == tid; });

		if (iter != m_threadIds.end())
		{
			delete *iter;
			*iter = nullptr;
		}
	}

	void ThreadPool::worker(ThreadPool* pool)
	{
		while (true)
		{
			std::unique_lock<std::mutex> locker(pool->m_mutex);

			//1.当任务队列为空且线程池未关闭时,阻塞工作线程
			while (pool->m_taskQueue->getTaskCount() == 0 && !pool->m_shutdown)
			{
				{
					std::lock_guard<std::mutex> outputLocker(m_outputMutex);
					std::cout << "Thread " << std::this_thread::get_id() << " is waiting..." << std::endl;
				}

				pool->cond.wait(locker);
			}

			// 2. 线程池关闭：退出当前工作线程
			if (pool->m_shutdown)
			{
				{
					std::lock_guard<std::mutex> outputLocker(m_outputMutex);
					std::cout << "Thread " << std::this_thread::get_id() << " is shutting down." << std::endl;
				}
				return;
			}

			// 3. 从队列中取出任务并执行

			auto task = pool->m_taskQueue->takeTask();
			pool->m_busyThreads++;
			locker.unlock();

			//执行任务
			{
				std::lock_guard<std::mutex> outputLocker(m_outputMutex);
				std::cout << "Thread " << std::this_thread::get_id() << " is working." << std::endl;
			}
			task();

			locker.lock();
			pool->m_busyThreads--;
		}
	}

	void ThreadPool::manger(ThreadPool* pool)
	{
		while (!pool->m_shutdown)
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));
			std::unique_lock<std::mutex> locker(pool->m_mutex);

			size_t task_size = pool->m_taskQueue->getTaskCount();
			size_t alive_num = pool->m_aliveThreads;
			size_t busy_num = pool->m_busyThreads;

			//1. 当任务数过多，线程池中的alive线程较小不够用时，创建线程
			//创建线程的条件：任务数task > 存活线程数(表示线程池不够用，需要扩大线程池），且存活线程数 < 最大线程数(表示线程池还能扩大)
			if (task_size > alive_num && alive_num < pool->m_maxThreads)
			{
				int count = 0; //记录创建线程数量
				size_t thread_size = pool->m_threadIds.size();
				for (size_t i = 0; i < thread_size && count < pool->ThreadByManger; ++i)
				{
					if (pool->m_threadIds[i] == nullptr)
					{
						pool->m_threadIds.emplace_back(new std::thread(&ThreadPool::worker, pool));
						{
							std::lock_guard<std::mutex> outputLocker(m_outputMutex);
							std::cout << "Manager created thread ID: " << pool->m_threadIds[i]->get_id() << std::endl;
						}
						count++;
						pool->m_aliveThreads++;
					}
				}
			}
		}
	}
}
