#include "mutex.hpp"

#include <twist/support/random.hpp>

#include <twist/fault/adversary/adversary.hpp>
#include <twist/fault/adversary/inject_fault.hpp>

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/executor.hpp>

#include <atomic>
#include <chrono>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

namespace stress {
  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          start_barrier_(parameters.Get(0)) {
    }

    // One-shot
    void Run() {
      twist::test_utils::ScopedExecutor executor;
      for (size_t t = 0; t < parameters_.Get(0); ++t) {
        executor.Submit(&Tester::RunLockThread, this);
      }
    }

   private:
    void RunLockThread() {
      start_barrier_.PassThrough();

      size_t iterations = parameters_.Get(1);
      for (size_t i = 0; i < iterations; ++i) {
        mutex_.Lock();
        CriticalSection();
        mutex_.Unlock();
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
    solutions::Mutex mutex_;
  };

};

void StressTest(TTestParameters parameters) {
  stress::Tester(parameters).Run();
}

// Parameters: threads, iterations

T_TEST_CASES(StressTest)
  .TimeLimit(std::chrono::seconds(30))
  .Case({2, 100000})
  .Case({10, 100000});

#if defined(TWIST_FIBER)

T_TEST_CASES(StressTest)
  .TimeLimit(std::chrono::seconds(30))
  .Case({10, 1000000});

#endif

////////////////////////////////////////////////////////////////////////////////

namespace wakeup {

void Test(size_t threads) {
  solutions::Mutex mutex;
  twist::test_utils::OnePassBarrier barrier{threads};

  auto contender = [&]() {
    barrier.PassThrough();

    mutex.Lock();
    mutex.Unlock();
  };

  twist::test_utils::ScopedExecutor executor;
  for (size_t i = 0; i < threads; ++i) {
    executor.Submit(contender);
  }
}

}  // namespace wakeup

void MissedWakeupTest(TTestParameters parameters) {
  size_t threads = parameters.Get(0);
  size_t iterations = parameters.Get(1);
 
  for (size_t i = 0; i < iterations; ++i) {
    wakeup::Test(threads);
  }
}

T_TEST_CASES(MissedWakeupTest)
  .TimeLimit(std::chrono::minutes(1))
  .Case({2, 1000})
  .Case({3, 1000});

#if defined(TWIST_FIBER)

T_TEST_CASES(MissedWakeupTest)
  .TimeLimit(std::chrono::minutes(1))
  .Case({2, 100000})
  .Case({3, 100000});	
 
#endif

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()

