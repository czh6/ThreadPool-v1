#include "Thread.h"
#include <thread>

int Thread::m_generateId = 0;

// 线程构造
Thread::Thread(ThreadFunc func) : m_func(func), m_threadId(m_generateId++)
{

}

// 线程析构
Thread::~Thread()
{

}

// 启动线程
void Thread::start()
{
	std::thread t(m_func, m_threadId); // 创建一个线程来执行一个线程函数
	t.detach(); // 设置分离线程
}

// 获取线程id
int Thread::getId() const
{
	return m_threadId;
}