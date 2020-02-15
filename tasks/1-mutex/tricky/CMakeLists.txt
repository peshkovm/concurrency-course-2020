cmake_minimum_required(VERSION 3.5)

begin_task()
set_task_sources(tricky_lock.hpp)
add_task_test(test test.cpp)
end_task()

