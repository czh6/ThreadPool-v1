#ifndef TASK_H
#define TASK_H

// 前置声明
class Result;
class Any;

// 任务抽象基类
class Task
{
public:
	// 任务构造
	Task();

	// 任务析构
	~Task() = default;
	
	// 把run()方法的返回值通过setVal()方法给到Result对象的成员变量m_any
	void exec();

	// 给成员变量m_presult指针赋值
	void setResult(Result* res);

	// 用户自定义任务类型，从Task继承，重写run()方法，实现自定义任务处理
	virtual Any run() = 0;

private:
	Result* m_presult; // Result对象的生命周期比Task对象的生命周期长
};

#endif