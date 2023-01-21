#ifndef ANY_H
#define ANY_H

#include <memory>

// Any类型：可以接收任意类型的数据
class Any
{
public:
	Any() = default;
	~Any() = default;
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	// 这个构造函数可以让Any类型接收任意类型的数据
	template <typename T>
	Any(T data) : m_upbase(std::make_unique<Derive<T>>(data))
	{

	}

	// 这个方法能把Any对象里面存储的data数据提取出来
	template <typename T>
	T any_cast()
	{
		Derive<T>* pd = dynamic_cast<Derive<T>*>(m_upbase.get());
		if (pd == nullptr)
		{
			throw "type mismatch!";
		}
		return pd->m_data;
	}

private:
	// 基类类型
	class Base
	{
	public:
		virtual ~Base() = default;
	};

	// 派生类类型
	template <typename T>
	class Derive : public Base
	{
	public:
		Derive(T data) : m_data(data)
		{

		}
		T m_data; // 保存任意类型的数据
	};

private:
	// 定义一个基类的指针
	std::unique_ptr<Base> m_upbase;
};

#endif