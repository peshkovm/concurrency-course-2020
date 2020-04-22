#pragma once

#include <tinyfutures/executors/task.hpp>

namespace tiny::executors {

void SafelyExecuteHere(Task& task);

}  // namespace tiny::executors
