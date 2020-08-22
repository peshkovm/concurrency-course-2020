#include "scheduler.hpp"
#include <twist/stdlike/thread.hpp>

namespace tinyfiber {

//////////////////////////////////////////////////////////////////////

static thread_local Scheduler* current_scheduler;

Scheduler* GetCurrentScheduler() {
  TINY_VERIFY(current_scheduler, "not in fiber context");
  return current_scheduler;
}

struct SchedulerScope {
  SchedulerScope(Scheduler* scheduler) {
    TINY_VERIFY(!current_scheduler,
                "cannot run scheduler from another scheduler");
    current_scheduler = scheduler;
  }

  ~SchedulerScope() {
    current_scheduler = nullptr;
  }
};

//////////////////////////////////////////////////////////////////////

Scheduler::Scheduler() {
}

Fiber* Scheduler::GetCurrentFiber() {
  TINY_VERIFY(running_ != nullptr, "Not in fiber context");
  return running_;
}

Fiber* Scheduler::GetAndResetCurrentFiber() {
  Fiber* current = running_;
  running_ = nullptr;
  return current;
}

void Scheduler::SetCurrentFiber(Fiber* fiber) {
  running_ = fiber;
}

// Operations invoked by running fibers

void Scheduler::SwitchToScheduler() {
  Fiber* caller = GetAndResetCurrentFiber();
  caller->Context().SwitchTo(loop_context_);
}

// System calls

void Scheduler::Spawn(FiberRoutine routine) {
  auto* created = CreateFiber(routine);
  Schedule(created);
}

void Scheduler::Yield() {
  Fiber* caller = GetCurrentFiber();
  caller->SetState(FiberState::Runnable);
  SwitchToScheduler();
}

void Scheduler::SleepFor(Duration duration) {
  Fiber* caller = GetCurrentFiber();
  sleep_queue_.PutFiberSleepFor(caller, duration);
  caller->SetState(FiberState::Sleeping);
  SwitchToScheduler();
}

void Scheduler::WakeUpFiber(Fiber* fiber) {
  fiber->SetState(FiberState::Runnable);
  run_queue_.PushBack(fiber);
}

void Scheduler::Terminate() {
  Fiber* caller = GetCurrentFiber();
  caller->SetState(FiberState::Terminated);
  SwitchToScheduler();
}

// Scheduling

void Scheduler::Run(FiberRoutine init) {
  SchedulerScope scope(this);
  Spawn(init);
  RunLoop();
}

Fiber* Scheduler::GetNextFiber() {
  if (sleep_queue_.IsAnyoneReadyToWakeUp()) {
    WakeUpFiber(sleep_queue_.TakeReadyToWakeUpFiber());
  } else if (run_queue_.IsEmpty()) {
    twist::stdlike::this_thread::sleep_for(
        Duration(sleep_queue_.MinSleepTime()));
    WakeUpFiber(sleep_queue_.TakeReadyToWakeUpFiber());
  }
  return run_queue_.PopFront();
}

void Scheduler::RunLoop() {
  while (!run_queue_.IsEmpty() || !sleep_queue_.IsEmpty()) {
    Fiber* next = GetNextFiber();
    SwitchTo(next);
    Reschedule(next);
  }
}

void Scheduler::SwitchTo(Fiber* fiber) {
  SetCurrentFiber(fiber);
  fiber->SetState(FiberState::Running);
  // Scheduler loop_context_ -> fiber->context_
  loop_context_.SwitchTo(fiber->Context());
}

void Scheduler::Reschedule(Fiber* fiber) {
  switch (fiber->State()) {
    case FiberState::Runnable:  // From Yield
      Schedule(fiber);
      break;
    case FiberState::Sleeping:  // From Sleep
      // do nothing
      break;
    case FiberState::Terminated:  // From Terminate
      Destroy(fiber);
      break;
    default:
      TINY_PANIC("Unexpected fiber state");
      break;
  }
}

void Scheduler::Schedule(Fiber* fiber) {
  run_queue_.PushBack(fiber);
}

Fiber* Scheduler::CreateFiber(FiberRoutine routine) {
  return Fiber::Create(routine);
}

void Scheduler::Destroy(Fiber* fiber) {
  delete fiber;
}

//////////////////////////////////////////////////////////////////////

Fiber* GetCurrentFiber() {
  return GetCurrentScheduler()->GetCurrentFiber();
}

}  // namespace tinyfiber
