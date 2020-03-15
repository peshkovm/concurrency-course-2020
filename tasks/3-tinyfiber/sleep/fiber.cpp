#include "fiber.hpp"
#include "scheduler.hpp"

#include <tinysupport/compiler.hpp>
#include <tinysupport/exception.hpp>

namespace tinyfiber {

//////////////////////////////////////////////////////////////////////

static FiberId GenerateId() {
  static FiberId next_id = 0;
  return ++next_id;
}

Fiber::Fiber(FiberRoutine routine, Stack&& stack, FiberId id)
    : routine_(std::move(routine)),
      stack_(std::move(stack)),
      state_(FiberState::Starting),
      id_(id) {
}

Fiber* Fiber::Create(FiberRoutine routine) {
  auto stack = Stack::Allocate();
  FiberId id = GenerateId();

  Fiber* fiber = new Fiber(std::move(routine), std::move(stack), id);

  fiber->SetupTrampoline();

  return fiber;
}

//////////////////////////////////////////////////////////////////////

static void FiberTrampoline() {
  // Fiber execution starts here

  Fiber* self = GetCurrentFiber();

  self->SetState(FiberState::Running);

  auto routine = self->UserRoutine();
  try {
    routine();
  } catch (...) {
    TINY_PANIC("Uncaught exception in fiber: " << CurrentExceptionMessage());
  }

  GetCurrentScheduler()->Terminate();  // never returns

  TINY_UNREACHABLE();
}

void Fiber::SetupTrampoline() {
  context_.Setup(
      /*stack=*/stack_.AsMemSpan(),
      /*trampoline=*/FiberTrampoline);
}

}  // namespace tinyfiber
