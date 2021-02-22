#include "coroutine.hpp"

namespace tinyfiber {
namespace coroutine {

static thread_local Coroutine* running_coroutine{nullptr};

Coroutine::Coroutine(Routine routine)
    : routine_(std::move(routine)), stack_(Stack::Allocate()) {
  // Not implemented
  SetupTrampoline();
}

void Coroutine::Resume() {
  // Not implemented
  if (Completed()) {
    throw CoroutineCompleted();
  }
  auto caller_coroutine = running_coroutine;
  running_coroutine = std::move(this);
  caller_context_.SwitchTo(coro_context_);
  running_coroutine = caller_coroutine;
  if (Exception() != nullptr) {
    std::rethrow_exception(Exception());
  }
}

void Coroutine::Suspend() {
  // Not implemented
  coro_context_.SwitchTo(caller_context_);
}

bool Coroutine::IsCompleted() const {
  // Not implemented
  return Completed();
}

void Suspend() {
  // Not implemented
  if (running_coroutine == nullptr) {
    throw NotInCoroutine();
  }
  GetCurrentCoroutine()->Suspend();
}

Coroutine* GetCurrentCoroutine() {
  return running_coroutine;
}

void Coroutine::CoroutineTrampoline() {
  // Coroutine execution starts here

  Coroutine* self = GetCurrentCoroutine();

  try {
    auto routine = self->UserRoutine();
    routine();
  } catch (...) {
    auto exception = std::current_exception();
    self->SetException(exception);
  }
  self->SetCompleted(true);
  self->coro_context_.SwitchTo(self->caller_context_);
}

void Coroutine::SetupTrampoline() {
  coro_context_.Setup(stack_.AsMemSpan(), CoroutineTrampoline);
}

}  // namespace coroutine
}  // namespace tinyfiber
