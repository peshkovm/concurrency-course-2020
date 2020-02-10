cmake_minimum_required(VERSION 3.5)

enable_language(ASM)

begin_task()
set_task_sources(jump.hpp jump.S)
add_task_test(unit_test unit_test.cpp)
end_task()
