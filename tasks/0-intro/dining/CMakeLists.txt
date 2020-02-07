cmake_minimum_required(VERSION 3.5)

begin_task()
set_task_sources(dining.cpp dining.hpp philosopher.cpp philosopher.hpp)
add_task_test(stress_test stress_test.cpp)
end_task()
