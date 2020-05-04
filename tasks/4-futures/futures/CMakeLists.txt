cmake_minimum_required(VERSION 3.9)

begin_task()
add_task_library(tinyfutures tinysupport twist asio-no-boost)
add_task_test_dir(tests)
end_task()
