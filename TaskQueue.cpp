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
		if (!task_queue.empty())
		{
			Task task = std::move(task_queue.front());
			task_queue.pop();
			return task;
		}
		return nullptr;
	}
}
