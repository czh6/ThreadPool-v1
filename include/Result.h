#ifndef RESULT_H
#define RESULT_H

#include <atomic>
#include "Any.hpp"
#include "Semaphore.hpp"

// 前置声明
class Task;

// 任务执行完成后的返回值类型
class Result
{
public:
	// 返回值构造函数
	Result(std::shared_ptr<Task> task, bool isValid = true);
	
	// 返回值析构函数
	~Result() = default;

	// setVal()方法将任务run()方法执行完的返回值赋给成员变量m_any
	void setVal(Any any);

	// 用户调用get()方法获取任务的返回值
	Any get();

private:
	Any m_any;                       // 存储任务的返回值
	Semaphore m_sem;                 // 信号量
	std::shared_ptr<Task> m_sptask;  // 指向待获取返回值的任务对象
	std::atomic_bool m_isValid;      // 返回值是否有效
};

#endif