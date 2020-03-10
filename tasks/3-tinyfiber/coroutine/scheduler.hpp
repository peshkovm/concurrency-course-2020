#pragma once

#include <vector>
#include <thread>
#include <functional>

#include <asio.hpp>

namespace tinyfiber {

class Scheduler {
 public:
  using Task = std::function<void()>;

  Scheduler(size_t thread_count)
      : thread_count_(thread_count) {
  }

  // Submit first task and run processing loop from multiple threads
  void Run(Task init);

  // Submit new task for execution
  void Submit(Task task);

  // Access current executor from task
  static Scheduler* Current();


 private:
  void RunWorkerThreads();
  void Work();

 private:
  size_t thread_count_;
  asio::io_context io_context_;
};

}  // namespace tinyfiber
