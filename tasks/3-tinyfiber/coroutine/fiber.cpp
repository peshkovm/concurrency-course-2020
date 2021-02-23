#include "fiber.hpp"

#include "coroutine.hpp"

namespace tinyfiber {

void Spawn(FiberRoutine routine, ThreadPool& thread_pool) {
  // Not implemented
  thread_pool.Submit([&]() {
    auto coro = new coroutine::Coroutine(routine);
    coro->Resume();
  });
}

void Spawn(FiberRoutine routine) {
  // Not implemented
  auto coro = new coroutine::Coroutine(routine);
  coro->Resume();
}

void Yield() {
  // Not implemented
  auto coro = coroutine::GetCurrentCoroutine();

  ThreadPool::Current()->SubmitContinuation([&]() { coro->Resume(); });
  coro->Suspend();
}

}  // namespace tinyfiber
