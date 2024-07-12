#pragma once
#include "TaskQueue.h"

namespace threadpool
{
	class ThreadPool
	{
	public:
		ThreadPool(int min, int max);
		~ThreadPool();

		inline void add_task(taskqueue::Task task);

		inline size_t getBusyThreadNum();

		inline size_t getAliveThreadNum();

	private:
		//�����̺߳���

		//�߳�����
		void threadExit();

		//ȡ������������ִ��
		static void worker(void* arg);

		//�����߳�
		static void manger();

		std::mutex m_mutex;
		std::condition_variable cond;
		std::vector<std::thread*> m_threadIds;
		std::thread m_mangerId;
		taskqueue::TaskQueue* m_taskQueue;

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
