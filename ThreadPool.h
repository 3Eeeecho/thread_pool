#pragma once
#include "TaskQueue.h"

namespace threadpool
{
	class ThreadPool
	{
	public:
		ThreadPool(int min, int max);
		~ThreadPool();

		void addTask(taskqueue::Task task);

		size_t getBusyThreadNum();

		size_t getAliveThreadNum();

	private:
		//�����̺߳���

		//�߳�����
		void threadExit();

		//ȡ������������ִ��
		static void worker(ThreadPool* pool);

		//�����߳�
		static void manger(ThreadPool* pool);

		std::mutex m_mutex;
		std::condition_variable cond;
		std::thread m_mangerId;
		std::vector<std::thread*> m_threadIds;
		std::unique_ptr<taskqueue::TaskQueue> m_taskQueue;

		//�̳߳ز�������
		//�����̳߳ص�����,�̳߳ص����,�Ƿ�ر�,������ÿ�ο��Ƶ��߳�����
		int m_minThreads;
		int m_maxThreads;
		int m_busyThreads;
		int m_aliveThreads;
		int m_exitThreads;
		bool m_shutdown;
		static constexpr int ThreadByManger = 2;
	};
}
