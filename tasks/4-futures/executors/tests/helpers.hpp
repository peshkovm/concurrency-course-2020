#pragma once

#include <chrono>
#include <ctime>

namespace test_helpers {

////////////////////////////////////////////////////////////////////////////////

class StopWatch {
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

}  // namespace test_helpers
