#pragma once

#include "context.hpp"
#include "stack.hpp"

#include <exception>
#include <functional>

namespace tinyfiber {
namespace coroutine {

struct CoroutineCompleted : public std::runtime_error {
  CoroutineCompleted() : std::runtime_error("Coroutine completed") {
  }
};

struct NotInCoroutine : public std::runtime_error {
  NotInCoroutine() : std::runtime_error("Not in coroutine") {
  }
};

using Routine = std::function<void()>;

class Coroutine {
 public:
  Coroutine(Routine routine);

  // Transfers control to coroutine
  void Resume();

  // Suspends current coroutine and
  // transfers control back to caller
  void Suspend();

  bool IsCompleted() const;

  Routine UserRoutine() {
    return routine_;
  }

  bool Completed() const {
    return completed_;
  }

  void SetCompleted(bool completed) {
    completed_ = completed;
  }

  std::exception_ptr Exception() {
    return exception_;
  }

  void SetException(std::exception_ptr exception) {
    exception_ = exception;
  }

 private:
  static void CoroutineTrampoline();
  void SetupTrampoline();

 private:
  // Your code goes here
  Routine routine_;
  Stack stack_;
  ExecutionContext caller_context_;
  ExecutionContext coro_context_;
  bool completed_{false};
  std::exception_ptr exception_{nullptr};
};

void Suspend();
Coroutine* GetCurrentCoroutine();

}  // namespace coroutine
}  // namespace tinyfiber
