#pragma once

#include <tinyfutures/executors/executor.hpp>

namespace tiny::executors {

// Executes scheduled tasks immediately on the current thread
IExecutorPtr GetInlineExecutor();

};  // namespace tiny::executors
