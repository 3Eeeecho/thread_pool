#pragma once
#include "TaskQueue.h"


namespace threadpool
{
	inline std::mutex m_outputMutex; // 添加用于保护输出的互斥锁

	class ThreadPool
	{
	public:
		ThreadPool(int min, int max);
		~ThreadPool();

		void addTask(taskqueue::Task task);

		size_t getBusyThreadNum();

		size_t getAliveThreadNum();

	private:
		//工作线程函数

		//线程销毁
		void threadExit();

		//取出队列中任务并执行
		static void worker(ThreadPool* pool);

		//管理线程
		static void manger(ThreadPool* pool);

		std::mutex m_mutex;
		std::condition_variable cond;
		std::thread m_mangerId;
		std::vector<std::thread*> m_threadIds;
		std::unique_ptr<taskqueue::TaskQueue> m_taskQueue;

		//线程池参数设置
		//包括线程池的容量,线程池的情况,是否关闭,管理者每次控制的线程数量
		int m_minThreads;
		int m_maxThreads;
		int m_busyThreads;
		int m_aliveThreads;
		int m_exitThreads;
		bool m_shutdown;
		static constexpr int ThreadByManger = 2;
	};
}
