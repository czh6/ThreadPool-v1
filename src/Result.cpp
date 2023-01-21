#include "Result.h"
#include "Task.h"

// 返回值构造函数
Result::Result(std::shared_ptr<Task> sptask, bool isValid) : m_isValid(isValid), m_sptask(sptask)
{
	m_sptask->setResult(this);
}

// 用户调用get()方法获取任务的返回值
Any Result::get()
{
	if (!m_isValid) return "";
	m_sem.wait(); // 如果任务没有执行完，这里会阻塞用户的线程
	return std::move(m_any);
}

// setVal()方法将任务run()方法执行完的返回值赋给成员变量m_any
void Result::setVal(Any any)
{
	this->m_any = std::move(any); // 存储任务run()方法执行完的返回值
	m_sem.post(); // 已经获取到任务的返回值，增加信号量资源
}