#include <iostream>
#include "TaskQueue.h"

int main()
{
	TaskQueue tq;
	std::cout << "Task number: " << tq.getTaskCount() << std::endl;

	// 添加一个lambda表达式作为任务
	tq.addTask([] { std::cout << "I'm task1\n"; });

	std::cout << "Task number after adding: " << tq.getTaskCount() << std::endl;

	try
	{
		Task fun = tq.takeTask();
		fun(); // 执行任务
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception occurred: " << e.what() << std::endl;
	}

	std::cout << "Task number after consuming: " << tq.getTaskCount() << std::endl;

	return 0;
}
