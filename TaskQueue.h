#pragma once
#include <functional>
#include <mutex>
#include <queue>
using Task = std::function<void()>;

class TaskQueue
{
public:
	TaskQueue(){}
	~TaskQueue() = default;

	//������,������񵽶�����
	void addTask(Task task);

	//������,�Ӷ�����ȡ����
	Task takeTask();

	//��ȡ������������
	auto getTaskCount()->size_t { return task_queue.size(); }

private:
	//������ɵ��ö��󣨺�����lambda���ʽ�ȣ������std::function������������
	std::queue<Task> task_queue;
	std::mutex m_mutex;
};



