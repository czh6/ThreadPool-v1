# 指明了对cmake的最低版本的要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本号
project(ThreadPool VERSION 1.0)

# 设置为ON，表示必须使用CMAKE_CXX_STANDARD指定的版本
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定C++17版本
set(CMAKE_CXX_STANDARD 17)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置最终可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 加载子目录
add_subdirectory(src)