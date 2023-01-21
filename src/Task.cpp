#include "Task.h"
#include "Result.h"

// 任务构造
Task::Task() : m_presult(nullptr)
{

}

// 把run()方法的返回值通过setVal()方法给到Result对象的成员变量m_any
void Task::exec()
{
	if (m_presult != nullptr)
	{
		m_presult->setVal(run()); // 这里发生多态调用
	}
}

// 给成员变量m_presult指针赋值
void Task::setResult(Result* res)
{
	m_presult = res;
}