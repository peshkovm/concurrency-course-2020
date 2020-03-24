#pragma once

#include <tinysupport/time.hpp>

#include <functional>

namespace tinyfiber {

//////////////////////////////////////////////////////////////////////

using FiberRoutine = std::function<void()>;

using FiberId = size_t;

//////////////////////////////////////////////////////////////////////

// Runs 'init' routine in fiber scheduler in the current thread
void RunScheduler(FiberRoutine init);

//////////////////////////////////////////////////////////////////////

// This fiber functions

// Starts a new fiber managed by the current scheduler and
// puts this fiber to the end of the run queue.
// Does not transfer control to the scheduler.
void Spawn(FiberRoutine routine);

// Transfers control to the current scheduler
// and puts the current fiber to the end of the run queue
void Yield();

// Blocks the execution of the current fiber for at least
// the specified 'duration'
void SleepFor(Duration duration);

// Returns the id of the current fiber
FiberId GetFiberId();

}  // namespace tinyfiber
