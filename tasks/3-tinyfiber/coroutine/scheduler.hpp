#pragma once

#include <asio.hpp>

#include <vector>
#include <thread>

namespace tinyfiber {

// Simple thread pool
class Scheduler {
 public:
  using Task = std::function<void()>;

  Scheduler(size_t thread_count);
  ~Scheduler();

  // Submit new task for execution
  void Submit(Task task);

  // Access current executor from task
  static Scheduler* Current();

  // No more tasks
  void Shutdown();

 private:
  void StartWorkerThreads(size_t thread_count);
  void Work();

 private:
  asio::io_context io_context_;
  // Some magic for graceful shutdown
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  // Running threads
  std::vector<std::thread> workers_;
};

}  // namespace tinyfiber
