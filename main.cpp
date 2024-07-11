#include <iostream>
#include "TaskQueue.h"
using namespace std;
int main()
{
    TaskQueue tq;
    cout << "Task number: " << tq.getTaskCount() << endl;

    // ���һ��lambda���ʽ��Ϊ����
    tq.addTask([] { cout << "I'm task1\n"; });

    cout << "Task number after adding: " << tq.getTaskCount() << endl;

    try {
        Task fun = tq.takeTask();
        fun(); // ִ������
    }
    catch (const exception& e) {
        cerr << "Exception occurred: " << e.what() << endl;
    }

    cout << "Task number after consuming: " << tq.getTaskCount() << endl;

    return 0;
}
