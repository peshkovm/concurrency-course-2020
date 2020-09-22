#pragma once

#include "api.hpp"
#include "fiber.hpp"

#include <tinysupport/time.hpp>

#include <asio.hpp>

namespace tinyfiber {

using FiberQueue = IntrusiveList<Fiber>;

class Scheduler {
 public:
  Scheduler();

  void Run(FiberRoutine init);

  // From fiber context

  void Spawn(FiberRoutine routine);
  void Yield();
  void SleepFor(Duration duration);
  void Terminate();

  Fiber* GetCurrentFiber();

 private:
  void RunLoop();
  Fiber* GetNextFiber();
  bool ShouldStopSleepContext();

  // Context switch: current fiber -> scheduler
  void SwitchToScheduler();
  // Context switch: scheduler -> fiber
  void SwitchTo(Fiber* fiber);

  void Reschedule(Fiber* fiber);
  void Schedule(Fiber* fiber);

  Fiber* CreateFiber(FiberRoutine routine);
  void Destroy(Fiber* fiber);

  void SetCurrentFiber(Fiber* fiber);
  Fiber* GetAndResetCurrentFiber();

 private:
  ExecutionContext loop_context_;
  FiberQueue run_queue_;
  asio::io_context sleep_context_;
  asio::executor_work_guard<asio::io_context::executor_type> work_ =
      asio::make_work_guard(sleep_context_);
  Fiber* running_{nullptr};
  int num_of_not_terminated_fibers_ = 0;
};

//////////////////////////////////////////////////////////////////////

Fiber* GetCurrentFiber();
Scheduler* GetCurrentScheduler();

}  // namespace tinyfiber
