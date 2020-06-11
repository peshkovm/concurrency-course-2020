#include <twist/test_framework/test_framework.hpp>

#include <twist/fiber/core/api.hpp>

using twist::fiber::RunScheduler;
using twist::fiber::Spawn;
using twist::fiber::Yield;

TEST_SUITE(TrickyLock) {
  // TrickyLock example for cooperative fibers
  SIMPLE_TEST(LiveLock) {
    static const size_t kFibers = 2;
    static const size_t kIterations = 100;

    size_t cs_count = 0;

    // TrickyLock state
    size_t thread_count = 0;

    // Put Yield-s to produce livelock
    auto routine = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        // Lock starts here
        while (thread_count++ > 0) {
          Yield();
          --thread_count;
        }
        // Acquired

        // Critical section starts here
        ++cs_count;
        ASSERT_TRUE_M(cs_count < 3, "Too many critical sections");
        // End of critical section

        // Unlock
        Yield();
        --thread_count;
        // Released
      }
    };

    auto main = [&]() {
      for (size_t k = 0; k < kFibers; ++k) {
        Spawn(routine);
      }
    };

    static const size_t kSwitchesRequired = 12345;

    RunScheduler(main, /*fuel=*/kSwitchesRequired);
  }
}

RUN_ALL_TESTS()
