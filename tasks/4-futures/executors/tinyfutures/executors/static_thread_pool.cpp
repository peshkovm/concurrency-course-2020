#include <tinyfutures/executors/static_thread_pool.hpp>

#include <tinyfutures/executors/thread_label.hpp>
#include <tinyfutures/executors/queues.hpp>
#include <tinyfutures/executors/helpers.hpp>

namespace tiny::executors {

IThreadPoolPtr MakeStaticThreadPool(size_t threads, const std::string& name) {
  return nullptr;  // Not implemented
}

}  // namespace tiny::executors
