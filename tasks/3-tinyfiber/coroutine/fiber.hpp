#pragma once

#include "coroutine.hpp"

namespace tinyfiber {

using FiberRoutine = coroutine::Routine;

void RunScheduler(FiberRoutine init);

void Spawn(FiberRoutine routine);
void Yield();

}  // namespace tinyfiber
