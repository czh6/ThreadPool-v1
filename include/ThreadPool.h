#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <unordered_map>
#include <memory>
#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

const int TASK_MAX_THRESHOLD = INT32_MAX;
const int THREAD_MAX_THRESHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60; // 单位：秒

// 线程池支持的工作模式
enum class PoolMode
{
	MODE_FIXED,     // 固定数量的线程
	MODE_CACHED,    // 线程数量可动态增长
};

// 前置声明
class Thread;
class Task;
class Result;

// 线程池类型
class ThreadPool
{
public:
	// 线程池构造
	ThreadPool();

	// 线程池析构
	~ThreadPool();

	// 设置线程池的工作模式
	void setMode(PoolMode mode);

	// 设置任务数量的上限阈值
	void setTaskSizeMaxThreshold(int threshhold);

	// 设置cached工作模式下线程数量的上限阈值
	void setThreadSizeMaxThreshold(int threshhold);

	// 给线程池提交任务：用户调用该接口，传入任务对象，即生产任务
	Result submitTask(std::shared_ptr<Task> sptask);

	// 开启线程池
	void start(int initThreadSize = std::thread::hardware_concurrency());

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	// 线程入口函数：线程池中的线程从任务队列中取任务，即消费任务
	void threadFunc(int threadid);

	// 检查线程池是否已经启动
	bool checkRunningState() const;

private:
	std::unordered_map<int, std::unique_ptr<Thread>> m_threads; // 线程列表

	int m_initThreadSize;               // 初始的线程数量
	int m_threadSizeMaxThreshold;       // cached工作模式下线程数量的上限阈值
	std::atomic_int m_curThreadSize;    // 当前线程的总数量
	std::atomic_int m_idleThreadSize;   // 空闲线程的数量

	std::queue<std::shared_ptr<Task>> m_taskQue;    // 任务队列
	std::atomic_int m_curTaskSize;                  // 当前任务的总数量
	int m_taskSizeMaxThreshold;                     // 任务数量的上限阈值

	std::mutex m_taskQueMutex;               // 保证任务队列线程安全的互斥锁
	std::condition_variable m_notFullCV;     // 等到任务队列不满
	std::condition_variable m_notEmptyCV;    // 等到任务队列不空
	std::condition_variable m_exitCV;        // 等到线程资源被全部回收

	PoolMode m_curPoolMode;              // 当前线程池的工作模式
	std::atomic_bool m_isPoolRunning;    // 线程池是否已经启动
};

#endif