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
			// ��ʼ���������
			m_taskQueue = std::make_unique<taskqueue::TaskQueue>();

			// ��ʼ���߳���
			m_threadIds.resize(m_maxThreads);

			for (int i = 0; i < m_minThreads; ++i)
			{
				m_threadIds[i] = new std::thread(&ThreadPool::worker, this);
				std::cout << "Created thread ID: " << m_threadIds[i]->get_id() << std::endl;
			}

			m_mangerId = std::thread(&ThreadPool::manger, this);
			std::cout << "Created Manager thread ID: " << m_mangerId.get_id() << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Exception in ThreadPool constructor: " << e.what() << std::endl;
			// ��������쳣��ȷ��������
			m_shutdown = true;
			if (m_mangerId.joinable())
				m_mangerId.join();
			for (int i = 0; i < m_threadIds.size(); ++i)
			{
				if (m_threadIds[i] && m_threadIds[i]->joinable())
					m_threadIds[i]->join();
				delete m_threadIds[i];
			}
			throw; // �����׳��쳣
		}
	}

	ThreadPool::~ThreadPool()
	{
		m_shutdown = true;

		if (m_mangerId.joinable())
		{
			std::cout << "Manager thread ID: " << m_mangerId.get_id() << " is exiting" << std::endl;
			m_mangerId.join();
		}

		// �ȴ������߳��˳�
		for (int i = 0; i < m_aliveThreads; ++i)
		{
			if (m_threadIds[i] && m_threadIds[i]->joinable())
			{
				std::cout << "Thread ID " << m_threadIds[i]->get_id() << " is exiting" << std::endl;
				m_threadIds[i]->join();
			}
			delete m_threadIds[i];
		}
	}

	void ThreadPool::addTask(taskqueue::Task task)
	{
		std::lock_guard locker(m_mutex);
		m_taskQueue->addTask(task);
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
			std::unique_lock locker(pool->m_mutex);

			//1.���������Ϊ�����̳߳�δ�ر�ʱ,���������߳�
			while (pool->m_taskQueue->getTaskCount() == 0 && !pool->m_shutdown)
			{
				std::cout << "thread " << std::this_thread::get_id() << " is waiting..." << std::endl;

				pool->cond.wait(locker);

				//�Ӵ��������ж��Ƿ���Ҫ�����߳�
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

			// 2. �̳߳عرգ��˳���ǰ�����߳�
			if (pool->m_shutdown)
			{
				std::cout << "Thread " << std::this_thread::get_id() << " is shutting down." << std::endl;
				return;
			}

			// 3. �Ӷ�����ȡ������ִ��
			auto task = pool->m_taskQueue->takeTask();
			pool->m_busyThreads++;


			//ִ������
			std::cout << "thread " << std::this_thread::get_id() << " is working." << std::endl;
			locker.unlock();
			task();

			//�������,��������
			locker.lock();
			std::cout << "Thread " << std::this_thread::get_id() << " has finished." << std::endl;
			pool->m_busyThreads--;
			locker.unlock();
		}
	}

	void ThreadPool::manger(ThreadPool* pool)
	{
		while (!pool->m_shutdown)
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));
			std::unique_lock locker(pool->m_mutex);
			size_t task_size = pool->m_taskQueue->getTaskCount();
			size_t alive_num = pool->m_aliveThreads;
			size_t busy_num = pool->m_busyThreads;

			//1. �����������࣬�̳߳��е�alive�߳̽�С������ʱ�������߳�
			//�����̵߳�������������task > ����߳���(��ʾ�̳߳ز����ã���Ҫ�����̳߳أ����Ҵ���߳��� < ����߳���(��ʾ�̳߳ػ�������)
			if (task_size > alive_num && alive_num < pool->m_maxThreads)
			{
				int count = 0; //��¼�����߳�����
				size_t thread_size = pool->m_threadIds.size();
				for (size_t i = 0; i < thread_size && count < pool->ThreadByManger; ++i)
				{
					if (pool->m_threadIds[i] == nullptr)
					{
						pool->m_threadIds[i] = new std::thread(&ThreadPool::worker, pool);
						std::cout << "manger create thread ID: " << pool->m_threadIds[i]->get_id() << std::endl;
						count++;
						pool->m_aliveThreads++;
					}
				}
			}

			//2. ���̳߳���æ���߳�����С���̳߳���������ˣ����Ҵ���߳���������С�߳���ʱ��˵����û����С�߳������������߳�
			//�����̵߳�������æ�߳���*2 < ����߳���(��ʾ�̳߳��������)���Ҵ���߳��� > ��С�߳���(��ʾ�̳߳ػ�����С)

			if (busy_num * 2 < alive_num && alive_num > pool->m_minThreads)
			{
				pool->m_exitThreads = pool->ThreadByManger;
				locker.unlock(); // �뿪�ٽ��������������߳��˳�
				pool->cond.notify_all(); // ���ѵȴ��Ĺ����߳�
			}
		}
	}
}
