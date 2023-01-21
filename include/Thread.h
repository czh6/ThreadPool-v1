#ifndef THREAD_H
#define THREAD_H

#include <functional>

// 线程类型
class Thread
{
public:
	// 线程入口函数类型
	using ThreadFunc = std::function<void(int)>;

	// 线程构造
	Thread(ThreadFunc func);

	// 线程析构
	~Thread();

	// 启动线程
	void start();

	// 获取线程id
	int getId() const;

private:
	ThreadFunc m_func;          // 保存线程的入口函数
	static int m_generateId;    // 生成一个线程id
	int m_threadId;             // 保存线程id
};

#endif