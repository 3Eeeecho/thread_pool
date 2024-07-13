#include <iostream>
#include "TaskQueue.h"
#include "ThreadPool.h"

//�̳߳����̵߳Ļص�����
void taskFunc()
{
	std::cout << "thread " << std::this_thread::get_id() << " is working " << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(2));
}

//�����̳߳�
void testThreadPool()
{
	// �����̳߳�
	threadpool::ThreadPool pool(3, 12);

	// ���̳߳������100�����񣬹۲��̳߳صĶ�̬������������ģʽ�Ĺ�����
	for (int i = 0; i < 10; i++)
	{
		pool.addTask(taskFunc);
	}

	std::this_thread::sleep_for(std::chrono::seconds(20)); //˯��40��,��ֹ���߳̽������̳߳�ִ�����٣���δ������񣨵ȴ��̳߳ش�����Task��
}

int main()
{
	testThreadPool(); //�����̳߳�
	return 0;
}
