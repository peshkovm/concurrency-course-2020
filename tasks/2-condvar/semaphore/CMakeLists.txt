cmake_minimum_required(VERSION 3.5)

begin_task()
set_task_sources(semaphore.hpp channel.hpp)
add_task_test(unit_test unit_test.cpp)
add_task_test(stress_test stress_test.cpp)
end_task()
