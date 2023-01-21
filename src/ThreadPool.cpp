#include "ThreadPool.h"
#include "Result.h"
#include "Thread.h"
#include "Task.h"
#include <iostream>
#include <chrono>

// 线程池构造
ThreadPool::ThreadPool()
	: m_initThreadSize(0)
	, m_threadSizeMaxThreshold(THREAD_MAX_THRESHOLD)
	, m_curThreadSize(0)
	, m_idleThreadSize(0)
	, m_curTaskSize(0)
	, m_taskSizeMaxThreshold(TASK_MAX_THRESHOLD)
	, m_curPoolMode(PoolMode::MODE_FIXED)
	, m_isPoolRunning(false)
{

}

// 线程池析构
ThreadPool::~ThreadPool()
{
	m_isPoolRunning = false;
	std::unique_lock<std::mutex> lock(m_taskQueMutex);
	m_notEmptyCV.notify_all();
	m_exitCV.wait(lock, [&]()->bool { return m_threads.size() == 0; }); // 等到线程资源被全部回收
}

// 设置线程池的工作模式
void ThreadPool::setMode(PoolMode mode)
{
	if (checkRunningState()) return;
	m_curPoolMode = mode;
}

// 设置任务数量的上限阈值
void ThreadPool::setTaskSizeMaxThreshold(int threshold)
{
	if (checkRunningState()) return;
	m_taskSizeMaxThreshold = threshold;
}

// 设置cached工作模式下线程数量的上限阈值
void ThreadPool::setThreadSizeMaxThreshold(int threshold)
{
	if (checkRunningState()) return;
	if (m_curPoolMode == PoolMode::MODE_CACHED)
	{
		m_threadSizeMaxThreshold = threshold;
	}
}

// 给线程池提交任务：用户调用该接口，传入任务对象，即生产任务
Result ThreadPool::submitTask(std::shared_ptr<Task> sptask)
{
	// 获取锁
	std::unique_lock<std::mutex> lock(m_taskQueMutex);

	// 用户提交任务，阻塞最长不能超过1s，否则判断提交任务失败，返回
	if (!m_notFullCV.wait_for(lock, std::chrono::seconds(1), [&]()->bool { return m_taskQue.size() < (size_t)m_taskSizeMaxThreshold; }))
	{
		// 走到这里表示m_notFullCV等待了1s，条件仍然没有满足
		std::cerr << "The task queue is full, and the task submission failed!" << std::endl;
		// 返回该任务的Result对象，false表示该任务的返回值无效
		return Result(sptask, false);
	}

	// 如果任务队列不满，则把任务放入任务队列中
	m_taskQue.emplace(sptask);
	m_curTaskSize++;

	// 因为新放了任务，所以任务队列肯定不空了，在m_notEmptyCV上进行通知，线程池中的线程赶快从任务队列中取任务，消费任务
	m_notEmptyCV.notify_all();

	// cached模式适合处理小而快的任务，需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
	if (m_curPoolMode == PoolMode::MODE_CACHED && m_curTaskSize > m_idleThreadSize && m_curThreadSize < m_threadSizeMaxThreshold)
	{
		std::cout << "cached: create a new thread!" << std::endl;
		// 创建新的线程对象
		auto upthread = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = upthread->getId();
		m_threads.emplace(threadId, std::move(upthread));
		// 启动线程
		m_threads[threadId]->start(); // 去执行线程入口函数
		// 修改线程个数相关的变量
		m_curThreadSize++;
		m_idleThreadSize++;
	}

	// 返回刚刚提交的任务的Result对象
	return Result(sptask);
}

// 开启线程池
void ThreadPool::start(int initThreadSize)
{
	// 设置线程池已经启动
	m_isPoolRunning = true;

	// 记录初始的线程数量
	m_initThreadSize = initThreadSize;

	// 创建线程对象
	for (int i = 0; i < m_initThreadSize; i++)
	{
		// 创建线程对象的时候，把线程入口函数给到线程对象
		auto upthread = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = upthread->getId();
		m_threads.emplace(threadId, std::move(upthread));
	}

	// 启动所有线程
	for (int i = 0; i < m_initThreadSize; i++)
	{
		m_threads[i]->start(); // 去执行线程入口函数
		// 修改线程个数相关的变量
		m_curThreadSize++;
		m_idleThreadSize++;
	}
}

// 线程入口函数：线程池中的线程从任务队列中取任务，即消费任务
void ThreadPool::threadFunc(int threadid)
{
	auto lastTime = std::chrono::high_resolution_clock().now();

	// 必须等所有任务执行完成，才可以回收所有线程资源
	for (;;)
	{
		std::shared_ptr<Task> sptask;

		{
			// 获取锁
			std::unique_lock<std::mutex> lock(m_taskQueMutex);

			std::cout << "threadid:" << std::this_thread::get_id() << " is trying!" << std::endl;

			while (m_taskQue.size() == 0)
			{
				// 线程池要结束，回收线程资源
				if (!m_isPoolRunning)
				{
					m_threads.erase(threadid);
					std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
					m_exitCV.notify_all();
					return; // 线程函数返回，相应的线程也就结束了
				}

				// cached模式下，有可能已经创建了很多线程，需要把空闲时间超过60s的多余线程结束回收掉（回收超过m_initThreadSize数量的线程）
				if (m_curPoolMode == PoolMode::MODE_CACHED)
				{
					// 条件变量m_notEmptyCV超时返回了
					if (m_notEmptyCV.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout)
					{
						auto nowTime = std::chrono::high_resolution_clock().now();
						auto duration = std::chrono::duration_cast<std::chrono::seconds>(nowTime - lastTime);
						if (duration.count() >= THREAD_MAX_IDLE_TIME && m_curThreadSize > m_initThreadSize)
						{
							m_threads.erase(threadid); // 回收当前线程，把线程对象从线程列表容器中删除
							m_curThreadSize--; // 修改线程个数相关的变量
							m_idleThreadSize--;
							std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
							return; // 线程函数返回，相应的线程也就结束了
						}
					}
				}
				else // fixed模式下，等到任务队列不空
				{
					m_notEmptyCV.wait(lock);
				}
			}

			m_idleThreadSize--;

			std::cout << "threadid:" << std::this_thread::get_id() << " is successful!" << std::endl;

			// 从任务队列中取一个任务出来
			sptask = m_taskQue.front();
			m_taskQue.pop();
			m_curTaskSize--;

			// 如果仍然有剩余任务，继续通知其它线程执行任务
			if (m_taskQue.size() > 0)
			{
				m_notEmptyCV.notify_all();
			}

			// 取出了一个任务，通知可以继续提交任务
			m_notFullCV.notify_all();
		} // 把锁释放掉

		// 当前线程负责执行取出来的这个任务
		if (sptask != nullptr)
		{
			sptask->exec(); // 执行任务，把run()方法的返回值通过setVal()方法给到Result对象的成员变量m_any
		}

		m_idleThreadSize++;
		lastTime = std::chrono::high_resolution_clock().now(); // 更新线程执行完任务的时间
	}
}

// 检查线程池是否已经启动
bool ThreadPool::checkRunningState() const
{
	return m_isPoolRunning;
}