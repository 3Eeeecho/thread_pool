#include "TaskQueue.h"

void TaskQueue::addTask(Task task)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    task_queue.push(task);
}

Task TaskQueue::takeTask()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (task_queue.empty()) {
        // �������Ϊ�գ������׳��쳣���߷���һ��������
        throw std::runtime_error("Task queue is empty");
    }

    Task task = task_queue.front();
    task_queue.pop();
    return task;
}

