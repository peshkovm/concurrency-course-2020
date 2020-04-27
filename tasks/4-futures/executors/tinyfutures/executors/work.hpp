#pragma once

#include <tinyfutures/executors/executor.hpp>

namespace tiny::executors {

// Extends _working_ time of underlying execution context
IExecutorPtr KeepWorking(IExecutorPtr e);

}  // namespace tiny::executors
