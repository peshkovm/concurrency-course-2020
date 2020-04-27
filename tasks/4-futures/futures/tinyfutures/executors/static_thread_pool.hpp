#pragma once

#include <tinyfutures/executors/thread_pool.hpp>

namespace tiny::executors {

// Fixed-size pool of threads + unbounded blocking queue
IThreadPoolPtr MakeStaticThreadPool(size_t threads, const std::string& name);

}  // namespace tiny::executors
