cmake_minimum_required(VERSION 3.5)

enable_language(ASM)

begin_task()
task_link_libraries(libtinysupport)
set_task_sources(
  context.hpp
  context.cpp
  context.S
  stack.hpp
  stack.cpp
  coroutine.hpp
  coroutine.cpp
  scheduler.hpp
  scheduler.cpp
  fiber.hpp
  fiber.cpp)
add_task_test(test test.cpp)
end_task()
