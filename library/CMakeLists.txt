cmake_minimum_required(VERSION 3.5)

# ------------------------------------------------------------------------------

add_library(asio-no-boost INTERFACE)
target_include_directories(asio-no-boost INTERFACE asio/asio/include)

# ------------------------------------------------------------------------------

add_subdirectory(twist)

# ------------------------------------------------------------------------------

add_subdirectory(tiny-support-v1)
add_subdirectory(tiny-support-v2)

# ------------------------------------------------------------------------------
