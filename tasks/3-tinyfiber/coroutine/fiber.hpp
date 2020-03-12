#pragma once

#include <functional>

namespace tinyfiber {

using FiberRoutine = std::function<void()>;

void RunScheduler(FiberRoutine init, size_t threads = 1);

void Spawn(FiberRoutine routine);
void Yield();

}  // namespace tinyfiber
