#pragma once

#include <tinysupport/memspan.hpp>

#include <cstdlib>
#include <cstdint>

namespace tinyfiber {

// TODO(Lipovsky): closure instead of void(void) function
typedef void (*Trampoline)();

struct ExecutionContext {
  // Execution context saved on top of suspended fiber/thread stack
  void* rsp_;

  // Prepare execution context for running trampoline function
  void Setup(MemSpan stack, Trampoline trampoline);

  // Save the current execution context to 'this' and jump to the
  // 'target' context. 'target' context created directly by Setup or
  // by another target.SwitchTo(other) call.
  void SwitchTo(ExecutionContext& target);

  // For testing purposes
  static size_t SwitchCount();
};

}  // namespace tinyfiber
