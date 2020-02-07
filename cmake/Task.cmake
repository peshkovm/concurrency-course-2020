set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

# --------------------------------------------------------------------

# Helpers

macro(get_task_target VAR NAME)
    set(${VAR} ${HW_NAME}_${TASK_NAME}_${NAME})
endmacro()

function(add_task_executable BINARY_NAME)
    prepend(BINARY_SOURCES "${TASK_DIR}/" ${ARGN})

    add_executable(${BINARY_NAME} ${BINARY_SOURCES} ${TASK_SOURCES})
    target_link_libraries(${BINARY_NAME} pthread twist)
    add_dependencies(${BINARY_NAME} twist)
endfunction()

# --------------------------------------------------------------------

# Prologue

macro(begin_task)
    set(TASK_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    get_filename_component(TASK_NAME ${TASK_DIR} NAME)

    get_filename_component(HW_DIR ${TASK_DIR} DIRECTORY)
    get_filename_component(HW_NAME ${HW_DIR} NAME)

    log_info("Homework = '${HW_NAME}', task = '${TASK_NAME}'")

    include_directories(${TASK_DIR})

    set(TEST_LIST "")
endmacro()

# --------------------------------------------------------------------

# Sources

macro(set_task_sources)
    prepend(TASK_SOURCES "${TASK_DIR}/" ${ARGV})
endmacro()

# --------------------------------------------------------------------

# Tests

function(add_task_test BINARY_NAME)
    get_task_target(TEST_NAME ${BINARY_NAME})
    add_task_executable(${TEST_NAME} ${ARGN})

    list(APPEND TEST_LIST ${TEST_NAME})
    set(TEST_LIST "${TEST_LIST}" PARENT_SCOPE)
endfunction()

function(add_task_all_tests_target)
    get_task_target(ALL_TESTS_TARGET "run_all_tests")
    run_chain(${ALL_TESTS_TARGET} ${TEST_LIST})
endfunction()

# --------------------------------------------------------------------

# Benchmark

function(add_task_benchmark BINARY_NAME)
    get_task_target(BENCH_NAME ${BINARY_NAME})
    add_task_executable(${BENCH_NAME} ${ARGN})
    target_link_libraries(${BENCH_NAME} benchmark)

    if(${TOOL_BUILD})
        get_task_target(RUN_BENCH_TARGET "run_benchmark")
        add_custom_target(${RUN_BENCH_TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BENCH_NAME})
        add_dependencies(${RUN_BENCH_TARGET} ${BENCH_NAME})
    endif()
endfunction()

# --------------------------------------------------------------------

# Epilogue

function(end_task)
    if(${TOOL_BUILD})
        add_task_all_tests_target()
    endif()
endfunction()
