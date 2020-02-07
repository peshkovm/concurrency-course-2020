cmake_minimum_required(VERSION 3.9)

include(cmake/Ccache.cmake)

project(tpcc_course)

include(cmake/Logging.cmake)
include(cmake/Helpers.cmake)
include(cmake/Platform.cmake)
include(cmake/CheckCompiler.cmake)
include(cmake/CompileOptions.cmake)
include(cmake/Sanitize.cmake)

add_subdirectory(library)

include(cmake/Task.cmake)
add_subdirectory(tasks)
