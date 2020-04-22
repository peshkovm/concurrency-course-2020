#pragma once

#include <tinyfutures/executors/executor.hpp>

namespace tiny::executors {

// Strand executes (via underlying executor) tasks
// non-concurrently and in the order they were added
IExecutorPtr MakeStrand(IExecutorPtr executor);

}  // namespace tiny::executors
