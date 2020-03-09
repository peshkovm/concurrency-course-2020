#pragma once

#include "context.hpp"
#include "stack.hpp"

#include "api.hpp"

#include <tinysupport/intrusive_list.hpp>

#include <asio.hpp>

namespace tinyfiber {

//////////////////////////////////////////////////////////////////////

enum class FiberState { Starting, Runnable, Running, Terminated };

class Fiber : public IntrusiveListNode<Fiber> {
 public:
  size_t Id() const {
    return id_;
  }

  ExecutionContext& Context() {
    return context_;
  }

  FiberState State() const {
    return state_;
  }

  void SetState(FiberState target) {
    state_ = target;
  }

  FiberRoutine UserRoutine() {
    return routine_;
  }

  static Fiber* Create(FiberRoutine routine);

 private:
  Fiber(FiberRoutine routine, Stack&& stack, FiberId id);

  void SetupTrampoline();

 private:
  FiberRoutine routine_;
  Stack stack_;
  ExecutionContext context_;
  FiberState state_;
  FiberId id_;
};

}  // namespace tinyfiber
