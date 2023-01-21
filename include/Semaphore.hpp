#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

// 信号量类
class Semaphore
{
public:
	// 信号量构造函数
	Semaphore(int limit = 0) : m_resLimit(limit)
	{

	}

	// 信号量析构函数
	~Semaphore() = default;

	// 获取一个信号量资源
	void wait()
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_cv.wait(lock, [&]()->bool { return m_resLimit > 0; }); // 如果没有资源，则会阻塞当前线程
		m_resLimit--;
	}

	// 增加一个信号量资源
	void post()
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_resLimit++;
		m_cv.notify_all(); // 通知条件变量wait的地方，可以起来干活了
	}

private:
	int m_resLimit;
	std::mutex m_mtx;
	std::condition_variable m_cv;
};

#endif