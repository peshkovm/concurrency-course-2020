#pragma once

#include <tinyfutures/executors/executor.hpp>

namespace tiny::executors {

using IExecutorWeakPtr = std::weak_ptr<IExecutor>;

// Extends _working_ time of underlying execution context

class WorkGuard {
 public:
  WorkGuard(IExecutorPtr e) : e_weak_(e) {
    e->WorkCreated();
  }

  WorkGuard(WorkGuard&& that) : e_weak_(that.Release()) {
  }

  WorkGuard& operator=(WorkGuard&& that) {
    CompleteWork();
    e_weak_ = that.Release();
    return *this;
  }

  // Non-copyable
  WorkGuard(const WorkGuard&) = delete;
  WorkGuard& operator=(const WorkGuard&) = delete;

  ~WorkGuard() {
    CompleteWork();
  }

 private:
  void CompleteWork() {
    if (auto e = e_weak_.lock()) {
      e->WorkCompleted();
    }
  }

 private:
  IExecutorWeakPtr&& Release() {
    return std::move(e_weak_);
  }

 private:
  IExecutorWeakPtr e_weak_;
};

WorkGuard MakeWorkFor(IExecutorPtr e);

}  // namespace tiny::executors
