#include "scheduler.hpp"

namespace tinyfiber {

void Scheduler::Run(Task init) {
  io_context_.reset();
  io_context_.post(init);
  RunWorkerThreads();
}

void Scheduler::Submit(Task task) {
  io_context_.post(task);
}

static thread_local Scheduler* current{nullptr};

Scheduler* Scheduler::Current() {
  return current;
}

void Scheduler::Work() {
  current = this;
  io_context_.run(); // invoke handlers
}

void Scheduler::RunWorkerThreads() {
  std::vector<std::thread> workers;

  for (size_t i = 0; i < thread_count_; ++i) {
    workers.emplace_back(std::bind(&Scheduler::Work, this));
  }

  for (auto &worker : workers) {
    worker.join();
  }
}

}  // namespace tinyfiber
