#include "fiber.hpp"

#include "coroutine.hpp"
#include "scheduler.hpp"

namespace tinyfiber {

void RunScheduler(FiberRoutine init, size_t threads) {
  Scheduler scheduler(threads);
  // Your code goes here
}

void Spawn(FiberRoutine routine) {
  // Not implemented
}

void Yield() {
  // Not implemented
}

}  // namespace tinyfiber
