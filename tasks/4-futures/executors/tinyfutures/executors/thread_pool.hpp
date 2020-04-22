#pragma once

#include <tinyfutures/executors/executor.hpp>

namespace tiny::executors {

struct IThreadPool : public IExecutor {
  // Graceful shutdown
  // Await all scheduled (directly or via decorating executors or
  // future pipelines) tasks and stop all threads
  virtual void Join() = 0;

  // Just stop as soon as possible ignoring all scheduled tasks
  virtual void Shutdown() = 0;

  virtual size_t ExecutedTaskCount() const = 0;

  virtual ~IThreadPool() = default;
};

using IThreadPoolPtr = std::shared_ptr<IThreadPool>;

}  // namespace tiny::executors
