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

 private:
  // Your code goes here
};

void Suspend();

}  // namespace coroutine
}  // namespace tinyfiber
