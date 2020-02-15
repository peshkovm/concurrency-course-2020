#include "spinlock.hpp"

#include <twist/support/random.hpp>

#include <twist/fault/adversary/adversary.hpp>
#include <twist/fault/adversary/inject_fault.hpp>

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/executor.hpp>

#include <twist/strand/spin_wait.hpp>

#include <atomic>
#include <chrono>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

namespace stress {
  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          start_barrier_(parameters.Get(0) + parameters.Get(1)) {
    }

    // One-shot
    void Run() {
      {
        twist::test_utils::ScopedExecutor executor;
        for (size_t t = 0; t < parameters_.Get(0); ++t) {
          executor.Submit(&Tester::RunTryLockThread, this);
        }
        for (size_t t = 0; t < parameters_.Get(1); ++t) {
          executor.Submit(&Tester::RunLockThread, this);
        }
      }
    }

   private:
    void RunLockThread() {
      start_barrier_.PassThrough();

      size_t iterations = parameters_.Get(2);
      for (size_t i = 0; i < iterations; ++i) {
        spinlock_.Lock();
        CriticalSection();
        spinlock_.Unlock();
      }
    }

    void RunTryLockThread() {
      start_barrier_.PassThrough();

      size_t iterations = parameters_.Get(2);
      for (size_t i = 0; i < iterations; ++i) {
        twist::strand::SpinWait spin_wait;
        while (!spinlock_.TryLock()) {
          spin_wait();
        }
        CriticalSection();
        spinlock_.Unlock();
      }
    }

    void CriticalSection() {
      ASSERT_FALSE(in_critical_section_.exchange(true));
      twist::fault::InjectFault();
      ASSERT_TRUE(in_critical_section_.exchange(false));
    }

   private:
    TTestParameters parameters_;

    twist::test_utils::OnePassBarrier start_barrier_;

    std::atomic<bool> in_critical_section_{false};
    solutions::SpinLock spinlock_;
  };

};

void StressTest(TTestParameters parameters) {
  stress::Tester(parameters).Run();
}

// Parameters: TryLock threads, Lock threads, iterations

T_TEST_CASES(StressTest)
  .TimeLimit(std::chrono::minutes(1))
  .Case({2, 2, 10000})
  .Case({10, 10, 50000});

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
