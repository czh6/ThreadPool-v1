#include <iostream>
#include <thread>
#include <chrono>
#include "ThreadPool.h"
#include "Task.h"
#include "Any.hpp"
#include "Result.h"

using uLong = unsigned long long;

class MyTask : public Task
{
public:
    MyTask(int begin, int end) : m_begin(begin), m_end(end)
    {

    }

    Any run()
    {
        std::cout << "threadid:" << std::this_thread::get_id() << " begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3)); // 模拟耗时的运算
        uLong sum = 0;
        for (uLong i = m_begin; i <= m_end; i++)
        {
            sum += i;
        }
        std::cout << "threadid:" << std::this_thread::get_id() << " end!" << std::endl;
        return sum;
    }

private:
    int m_begin, m_end;
};

int main()
{
    std::cout << "main begin!" << std::endl;

    {
        ThreadPool pool;
        pool.setMode(PoolMode::MODE_CACHED);
        pool.start(2);

        Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
        Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
        Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));

        // 随着task被执行完，task对象没了，依赖于task对象的Result对象也没了
        uLong sum1 = res1.get().any_cast<uLong>();
        uLong sum2 = res2.get().any_cast<uLong>();
        uLong sum3 = res3.get().any_cast<uLong>();

        std::cout << (sum1 + sum2 + sum3) << std::endl;
    }

    std::cout << "main end!" << std::endl;

    getchar();
    
    return 0;
}