#pragma once
#include "TaskQueue.h"
class ThreadPool
{
public:
	ThreadPool(int min, int max);
	~ThreadPool();

	void addTask(Task task);

	size_t getBusyThreadNum();

	size_t getAliveThreadNum();

private:
	//�����̺߳���
	//ȡ������������ִ��
	static void worker();

	//�����߳�
	static void manger();

	//�߳�����
	void threadExit();

private:
	std::mutex m_mutex;
	std::condition_variable cond;
	std::thread* m_threadIds;
	std::thread m_managerId;
	TaskQueue* m_taskQueue;

	//�̳߳ز�������
	//�����̳߳ص�����,�̳߳ص����,�Ƿ�ر�,������ÿ�ο��Ƶ��߳�����
	int m_minThreads;
	int m_maxThreads;
	int m_busyThreads;
	int m_aliveThreads;
	int m_exitThreads;
	bool m_shutdown;
	static const int ThreadByManger = 2;
};

