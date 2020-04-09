cmake_minimum_required(VERSION 3.5)

begin_task()
set_task_sources(cyclic_barrier.hpp)
add_task_test(unit_test unit_test.cpp)
add_task_test(stress_test stress_test.cpp)
add_task_benchmark(benchmark benchmark.cpp)
end_task()
