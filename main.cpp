#include <iostream>
#include "TaskQueue.h"
#include "ThreadPool.h"

//�̳߳����̵߳Ļص�����
void taskFunc()
{
	{
		std::lock_guard<std::mutex> locker(threadpool::m_outputMutex);
		std::cout << "Thread " << std::this_thread::get_id() << " is finished." << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::seconds(2));
}

//�����̳߳�
void testThreadPool()
{
	// �����̳߳�
	threadpool::ThreadPool pool(4, 16);

	// ���̳߳������100�����񣬹۲��̳߳صĶ�̬������������ģʽ�Ĺ�����
	for (int i = 0; i < 10; i++)
	{
		pool.addTask(taskFunc);
	}

	std::this_thread::sleep_for(std::chrono::seconds(10)); //˯��10��,��ֹ���߳̽������̳߳�ִ�����٣���δ������񣨵ȴ��̳߳ش�����Task��
}

int main()
{
	testThreadPool(); //�����̳߳�
	return 0;
}
