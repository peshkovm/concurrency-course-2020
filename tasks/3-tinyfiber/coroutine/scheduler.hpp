#pragma once

#include <asio.hpp>

#include <vector>
#include <thread>

namespace tinyfiber {

class ThreadPool {
 public:
  using Task = std::function<void()>;

  ThreadPool(size_t thread_count);
  ~ThreadPool();

  // Submit new task for execution
  void Submit(Task task);

  // Locate current thread pool from task
  static ThreadPool* Current();

  // No more tasks
  void Shutdown();

 private:
  void StartWorkerThreads(size_t thread_count);
  void Work();

 private:
  // Use io_context as task queue
  asio::io_context io_context_;
  // Some magic for graceful shutdown
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  // Worker threads
  std::vector<std::thread> workers_;
};

}  // namespace tinyfiber
