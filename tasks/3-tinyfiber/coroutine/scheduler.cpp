#include "scheduler.hpp"

#include <functional>

namespace tinyfiber {

ThreadPool::ThreadPool(size_t thread_count)
  : work_guard_(asio::make_work_guard(io_context_)) {
  StartWorkerThreads(thread_count);
}

ThreadPool::~ThreadPool() {
  Join();
}

void ThreadPool::Submit(Task task) {
  asio::post(io_context_, task);
}

static thread_local ThreadPool* current{nullptr};

ThreadPool* ThreadPool::Current() {
  return current;
}

void ThreadPool::Join() {
  if (joined_) {
    return;
  }
  work_guard_.reset();
  for (auto& worker : workers_) {
    worker.join();
  }
  joined_ = true;
}

void ThreadPool::Work() {
  current = this;
  io_context_.run(); // invoke posted handlers
}

void ThreadPool::StartWorkerThreads(size_t thread_count) {
  for (size_t i = 0; i < thread_count; ++i) {
    workers_.emplace_back(std::bind(&ThreadPool::Work, this));
  }
}

}  // namespace tinyfiber
