#pragma once

#include <tinyfutures/executors/task.hpp>

#include <memory>

namespace tiny::executors {

struct IExecutor {
  virtual void Execute(Task&& task) = 0;

  virtual void WorkCreated() = 0;
  virtual void WorkCompleted() = 0;

  virtual ~IExecutor() = default;
};

using IExecutorPtr = std::shared_ptr<IExecutor>;

}  // namespace tiny::executors
