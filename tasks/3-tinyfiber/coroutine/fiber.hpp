#pragma once

#include <functional>

#include "scheduler.hpp"

namespace tinyfiber {

using FiberRoutine = std::function<void()>;

// Spawn fiber inside provided thread pool
void Spawn(FiberRoutine routine, ThreadPool& thread_pool);

// Spawn fiber inside current thread pool
void Spawn(FiberRoutine routine);

void Yield();

}  // namespace tinyfiber
