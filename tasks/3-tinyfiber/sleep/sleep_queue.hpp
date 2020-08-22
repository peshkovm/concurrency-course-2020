#pragma once

#include "fiber.hpp"
#include <queue>

namespace tinyfiber {

class MyStopWatch {  // because start_time_ and Now() in original StopWatch is
  // private
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;

 public:
  MyStopWatch() : start_time_(Now()) {
  }

  Duration Elapsed() const {
    return Now() - start_time_;
  }

  Duration ElapsedFrom(TimePoint now) const {
    return now - start_time_;
  }

  static TimePoint Now() {
    return Clock::now();
  }

 private:
  TimePoint start_time_;
};

class SleepFiberNode {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;

  Duration RemainingSleepTime() const;

  Duration RemainingSleepTimeFrom(TimePoint from_time) const;

  bool IsReadyToWakedUp() const;

  Fiber* GetFiber() const;

  bool operator<(const SleepFiberNode& sleep_fiber) const;

  SleepFiberNode(Fiber* fiber, Duration duration);

 private:
  Fiber* fiber_;

  MyStopWatch stop_watch_;
  Duration duration_;
};

class SleepQueue : private std::priority_queue<SleepFiberNode> {
  // Not implemented
 public:
  void PutFiberSleepFor(Fiber* fiber, Duration duration);

  bool IsEmpty() const;

  Fiber* TakeReadyToWakeUpFiber();

  Duration MinSleepTime();

  bool IsAnyoneReadyToWakeUp();
};

}  // namespace tinyfiber
