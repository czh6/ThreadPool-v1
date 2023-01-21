# 1.线程池介绍

操作系统创建线程和销毁线程都是很“重”的操作，耗时、耗性能都比较多。在服务执行的过程中，如果业务量比较大，实时地去创建线程、执行业务、业务完成后销毁线程，那么会导致系统的实时性能降低，业务的处理能力也会降低。

线程池的优势：在服务进程启动之初，就事先创建好线程池里面的线程，当业务流量到来时需要分配线程，直接从线程池中获取一个空闲线程执行任务即可，任务执行完成后也不用释放线程，而是把线程归还到线程池中，继续给后续的任务提供服务。

# 2.开发环境

本项目的开发环境如下：
- Ubuntu Server 22.04.1
- gcc 11.3.0
- g++ 11.3.0
- make 4.3
- gdb 12.1
- cmake 3.22.1

开发环境的配置教程如下：
- [在 VMware Workstation 16 Pro 中安装 Ubuntu Server 22.04.1 并配置静态 IP 地址](https://blog.csdn.net/qq_42815188/article/details/128723376)
- [Ubuntu 配置 C/C++ 开发环境](https://blog.csdn.net/qq_42815188/article/details/128733561)
- [Windows 下 VS Code 远程连接 Ubuntu 并配置免密登录](https://blog.csdn.net/qq_42815188/article/details/128736694)

# 3.编译运行

在项目目录下，依次执行如下命令：

```cpp
mkdir build
cd build
cmake ..
make
cd ..
./threadpool
```

# 4.线程池的使用方法

具体的使用方法查看 `src/main.cpp`。

## 4.1 线程池支持的工作模式

本项目实现的线程池支持如下两种工作模式：
- fixed模式：线程池里面的线程个数是固定不变的，创建时根据当前机器的CPU核心数量进行指定。
- cached模式：线程池里面的线程个数是可动态增长的，根据任务的数量动态地增加线程的数量，但是会设置一个线程数量的阈值，任务处理完成，如果动态增长的线程空闲了60s还没有处理其他任务，则关闭线程，保持池中最初数量的线程即可。

示例如下：

```cpp
ThreadPool pool;
pool.setMode(PoolMode::MODE_CACHED); // 设置线程池的工作模式为cached模式
pool.start(2); // 开启线程池并设置初始的线程数量为2
```

## 4.2 往线程池中提交任务

首先需要继承任务抽象基类 `Task`，重写其中的 `run()` 方法，示例如下：

```cpp
class MyTask : public Task
{
public:
    MyTask(int begin, int end) : m_begin(begin), m_end(end)
    {

    }
    
    // 计算[begin,end]的数字之和
    Any run()
    {
        std::cout << "threadid:" << std::this_thread::get_id() << " begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
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
```

然后调用 `submitTask()` 方法往线程池中提交任务，等待任务执行完成并获取任务的返回值，示例如下：
- 自己实现了 `Any` 类（C++17的any），可以接收任意类型的数据；
- 自己实现了 `Semaphore` 类（C++20的semaphore），信号量用于线程之间的通信；
- 自己实现了 `Result` 类（C++11的future），等待任务执行完成并获取任务的返回值。

```cpp
Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));

uLong sum1 = res1.get().any_cast<uLong>();
uLong sum2 = res2.get().any_cast<uLong>();
uLong sum3 = res3.get().any_cast<uLong>();

std::cout << (sum1 + sum2 + sum3) << std::endl;
```
