#include <iostream>
#include "TaskQueue.h"
#include "ThreadPool.h"

//线程池中线程的回调函数
void taskFunc()
{
	std::cout << "thread " << std::this_thread::get_id() << " is working " << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(2));
}

//测试线程池
void testThreadPool()
{
	// 创建线程池
	threadpool::ThreadPool pool(3, 12);

	// 往线程池中添加100个任务，观察线程池的动态增长（管理者模式的工作）
	for (int i = 0; i < 10; i++)
	{
		pool.addTask(taskFunc);
	}

	std::this_thread::sleep_for(std::chrono::seconds(20)); //睡眠40秒,防止主线程结束后线程池执行销毁，尚未完成任务（等待线程池处理完Task）
}

int main()
{
	testThreadPool(); //测试线程池
	return 0;
}
