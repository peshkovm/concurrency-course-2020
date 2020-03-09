#include "scheduler.hpp"

#include <twist/test_framework/test_framework.hpp>

#include <twist/support/random.hpp>
#include <twist/support/time.hpp>

#include <cmath>
#include <ctime>
#include <iostream>
#include <functional>
#include <set>
#include <thread>

// Test utils

class ContextSwitchCounter {
 public:
  ContextSwitchCounter()
    : start_count_{tinyfiber::ExecutionContext::SwitchCount()} {
  }

  size_t Get() const {
    return tinyfiber::ExecutionContext::SwitchCount() - start_count_;
  }

 private:
  size_t start_count_;
};

class CPUTimer {
 public:
  CPUTimer() {
    start_ts_ = std::clock();
  }

  double SecondsElapsed() const {
    auto now_ts = std::clock();
    return 1.0 * (now_ts - start_ts_) / CLOCKS_PER_SEC;
  }

 private:
  std::clock_t start_ts_;
};

// Tests

TEST_SUITE(Scheduler) {
  SIMPLE_TEST(SleepFor) {
    static const auto kDelay = std::chrono::seconds(1);

    auto sleeper = []() {
      twist::Timer timer;
      tinyfiber::SleepFor(kDelay);
      ASSERT_TRUE(timer.Elapsed() > kDelay);
    };

    twist::Timer timer;
    tinyfiber::RunScheduler(sleeper);
    auto elapsed = timer.Elapsed();
    ASSERT_TRUE(elapsed > kDelay);
    ASSERT_TRUE(elapsed < kDelay + std::chrono::milliseconds(100));
  }

  SIMPLE_TEST(ConcurrentSleeps) {
    static const size_t kFibers = 100;

    auto launcher = [&]() {
      for (size_t i = 1; i <= kFibers; ++i) {
        auto sleeper = [i]() {
          tinyfiber::SleepFor(std::chrono::milliseconds(i * 10));
        };
        tinyfiber::Spawn(sleeper);
      }
    };

    twist::Timer timer;
    tinyfiber::RunScheduler(launcher);
    ASSERT_TRUE(timer.Elapsed() < std::chrono::milliseconds(1500));
  }

  SIMPLE_TEST(DontBurnCPU) {
    auto sleeper = []() {
      tinyfiber::SleepFor(std::chrono::seconds(1));
    };

    CPUTimer cpu_timer;
    ContextSwitchCounter switch_counter;

    tinyfiber::RunScheduler(sleeper);

    const auto cpu_time_seconds = cpu_timer.SecondsElapsed();
    const auto switch_count = switch_counter.Get();

    std::cout << "CPU time: " << cpu_time_seconds << " seconds" << std::endl;
    std::cout << "Switch count: " << switch_count << std::endl;

    ASSERT_TRUE(cpu_time_seconds < 0.1);
    ASSERT_TRUE(switch_count < 10);
  }

  SIMPLE_TEST(SleepAndRun) {
    size_t runner_steps = 0;

    auto runner = [&]() {
      twist::Timer timer;
      do {
        ++runner_steps;
        tinyfiber::Yield();
      } while (timer.Elapsed() < std::chrono::seconds(1));
    };

    auto sleeper = [&]() {
      tinyfiber::SleepFor(std::chrono::seconds(1));
      ASSERT_TRUE(runner_steps >= 1234);
    };

    auto main = [&]() {
      tinyfiber::Spawn(runner);
      tinyfiber::Spawn(sleeper);
    };

    tinyfiber::RunScheduler(main);
  }

  SIMPLE_TEST(SleepQueuePriority) {
    bool stop_requested = false;

    static const size_t kRunnerSteps = 1234;

    auto runner = [&]() {
      for (size_t i = 0; i < kRunnerSteps; ++i) {
        tinyfiber::Yield();
      }
      stop_requested = true;
    };

    auto sleeper = [&]() {
      size_t count = 0;
      while (!stop_requested) {
        ++count;
        tinyfiber::SleepFor(std::chrono::microseconds(1));
      }
    };

    tinyfiber::RunScheduler([&]() {
      for (size_t i = 0; i < 10; ++i) {
        tinyfiber::Spawn(sleeper);
      }
      tinyfiber::Spawn(runner);
    });
  }

  SIMPLE_TEST(RunQueuePriority) {
    bool stop_requested = false;

    auto runner = [&]() {
      while (!stop_requested) {
        tinyfiber::Yield();
      }
    };

    auto sleeper = [&]() {
      tinyfiber::SleepFor(std::chrono::seconds(2));
      stop_requested = true;
    };

    tinyfiber::RunScheduler([&]() {
      tinyfiber::Spawn(runner);
      tinyfiber::Spawn(sleeper);
    });
  }
}

RUN_ALL_TESTS()
