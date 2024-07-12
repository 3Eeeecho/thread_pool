#pragma once
#include <functional>
#include <mutex>
#include <queue>

namespace taskqueue
{
	using Task = std::function<void()>;

	class TaskQueue
	{
	public:
		TaskQueue()
		{
		}

		~TaskQueue() = default;

		//生产者,添加任务到队列中
		void addTask(Task task);

		//消费者,从队列中取队列
		Task takeTask();

		//获取队列中任务数
		size_t getTaskCount() { return task_queue.size(); }

	private:
		//将任意可调用对象（函数、lambda表达式等）打包成std::function对象放入队列中
		std::queue<Task> task_queue;
		std::mutex m_mutex;
	};
}
