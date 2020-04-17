cmake_minimum_required(VERSION 3.5)

begin_task()
task_link_libraries(asio-no-boost)
set_task_sources(server.hpp client.hpp)
add_task_test(test test.cpp)
end_task()
