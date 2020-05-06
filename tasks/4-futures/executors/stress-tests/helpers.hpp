#pragma once

#include <twist/test_framework/test_framework.hpp>

#include <chrono>
#include <ctime>

#include <mutex>
#include <condition_variable>

namespace test_helpers {

////////////////////////////////////////////////////////////////////////////////

class TestProgress {
 public:
  void IterCompleted() {
    ++i_;
    if (i_ % 256 == 0) {
      std::cout << i_ << "-ith iteration completed\n";
    }
  }

 private:
  size_t i_{0};
};

////////////////////////////////////////////////////////////////////////////////

class StopWatch {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using Duration = std::chrono::nanoseconds;

 public:
  StopWatch() : start_time_(Now()) {
  }

  Duration Elapsed() const {
    return Now() - start_time_;
  }

  Duration Restart() {
    auto elapsed = Elapsed();
    start_time_ = Now();
    return elapsed;
  }

 private:
  static TimePoint Now() {
    return Clock::now();
  }

 private:
  TimePoint start_time_;
};

class WallTimeBudget {
  using Duration = StopWatch::Duration;

 public:
  WallTimeBudget(Duration budget)
    : budget_(budget) {}

  bool Exhausted() const {
    return stop_watch_.Elapsed() > budget_;
  }

 private:
  StopWatch stop_watch_;
  Duration budget_;
};

class WallTimeLimitGuard {
  using Duration = StopWatch::Duration;
 public:
  WallTimeLimitGuard(Duration limit) : limit_(limit) {
  }

  ~WallTimeLimitGuard() {
    ASSERT_TRUE(stop_watch_.Elapsed() <= limit_);
  }

 private:
  StopWatch stop_watch_;
  Duration limit_;
};

////////////////////////////////////////////////////////////////////////////////

class CPUTimeMeter {
 public:
  CPUTimeMeter() {
    start_clocks_ = std::clock();
  }

  double UsageSeconds() const {
    return ClocksToSeconds(std::clock() - start_clocks_);
  }

 private:
  static double ClocksToSeconds(size_t clocks) {
    return 1.0 * clocks / CLOCKS_PER_SEC;
  }

 private:
  std::clock_t start_clocks_;
};

class CPUTimeBudgetGuard {
 public:
  CPUTimeBudgetGuard(double limit) : limit_(limit) {
  }

  ~CPUTimeBudgetGuard() {
    ASSERT_TRUE(meter_.UsageSeconds() <= limit_);
  }

 private:
  CPUTimeMeter meter_;
  double limit_;
};

////////////////////////////////////////////////////////////////////////////////

class OnePassBarrier {
 public:
  OnePassBarrier(size_t threads)
    : threads_(threads) {
  }

  void Arrive() {
    std::unique_lock lock(mutex_);
    --threads_;
    if (threads_ > 0) {
      all_arrived_.wait(lock, [this]() { return threads_ == 0; });
    } else {
      all_arrived_.notify_all();
    }
  }

 private:
  size_t threads_;
  std::mutex mutex_;
  std::condition_variable all_arrived_;
};

}  // namespace test_helpers
