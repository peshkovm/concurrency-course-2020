#include "scheduler.hpp"
#include "timer.hpp"

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
  // Intentionally ineffective implementation

  //  StopWatch stop_watch;
  //  do {
  //    Yield();
  //  } while (stop_watch.Elapsed() < duration);

  Fiber* fiber = GetCurrentFiber();
  fiber->SetState(FiberState::Sleeping);

  WaitableTimer timer(run_context_, duration);
  timer.async_wait([this, fiber](asio::error_code err) {
    // handler code
    if (!err) {
      fiber->SetState(FiberState::Runnable);
      SwitchTo(fiber);
      Reschedule(fiber);
    }
  });
  SwitchToScheduler();
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

void Scheduler::RunLoop() {
  while (!run_context_.stopped()) {
    run_context_.run();
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

void Scheduler::AddToQueue(Fiber* fiber) {
  run_context_.post([this, fiber]() {
    // handler code
    SwitchTo(fiber);
    Reschedule(fiber);
  });
}

void Scheduler::Schedule(Fiber* fiber) {
  //  run_queue_.PushBack(fiber);
  AddToQueue(fiber);
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
