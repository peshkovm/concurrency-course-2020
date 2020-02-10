cmake_minimum_required(VERSION 3.5)

begin_task()
set_task_sources(guarded.hpp)
add_task_test(unit_test unit_test.cpp)
end_task()
