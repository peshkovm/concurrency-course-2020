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
  api.cpp
  api.hpp
  fiber.hpp
  fiber.cpp
  scheduler.hpp
  scheduler.cpp
  awaiter.hpp
  socket.hpp
  socket.cpp
  echo.hpp
  echo.cpp)
add_task_test(test test.cpp)
end_task()
