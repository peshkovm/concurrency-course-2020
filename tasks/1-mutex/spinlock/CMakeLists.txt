cmake_minimum_required(VERSION 3.5)

enable_language(ASM)

begin_task()
set_task_sources(atomics.hpp atomics.S spinlock.hpp)
add_task_test(unit_test unit_test.cpp)
add_task_test(stress_test stress_test.cpp)
end_task()
