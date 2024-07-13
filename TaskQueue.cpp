#include "TaskQueue.h"

#include <iostream>

namespace taskqueue
{
	void TaskQueue::addTask(Task task)
	{
		std::lock_guard locker(m_mutex);
		task_queue.push(task);
	}

	Task TaskQueue::takeTask()
	{
		std::lock_guard locker(m_mutex);
		if (task_queue.empty())
		{
			// 如果队列为空，可以抛出异常或者返回一个空任务
			std::cerr << "queue is empty!" << std::endl;
			return nullptr;
		}

		Task task = task_queue.front();
		task_queue.pop();
		return task;
	}
}
