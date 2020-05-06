cmake_minimum_required(VERSION 3.9)

begin_task()
add_task_library(tinyfutures tinysupport twist)
add_task_test_dir(tests)
add_task_test_dir(stress-tests)
end_task()
