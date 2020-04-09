#include "cyclic_barrier.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/executor.hpp>
#include <twist/test_utils/barrier.hpp>

#include <vector>

////////////////////////////////////////////////////////////////////////////////

void RotatingLeaderStressTest(const TTestParameters& parameters) {
  size_t threads = parameters.Get(0);
  solutions::CyclicBarrier barrier{threads};
  size_t round = 0;

  auto routine = [&](size_t thread_index) {
    barrier.Arrive();

    size_t iterations = parameters.Get(1);
    for (size_t i = 0; i < iterations; ++i) {
      // Rotating leader writes to shared variable
      if (i % threads == thread_index) {
        round = i;
      } else {
        twist::strand::this_thread::yield();
      }

      barrier.Arrive();

      // All threads read from shared variable
      ASSERT_EQ(round, i);

      barrier.Arrive();
    }
  };

  twist::test_utils::ScopedExecutor executor;
  for (size_t i = 0; i < threads; ++i) {
    executor.Submit(routine, i);
  }
}

T_TEST_CASES(RotatingLeaderStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({2, 50000})
    .Case({5, 25000})
    .Case({10, 10000});

#if defined(TWIST_FIBER)

T_TEST_CASES(RotatingLeaderStressTest)
    .TimeLimit(std::chrono::seconds(10))
    .Case({10, 100000});

#endif

////////////////////////////////////////////////////////////////////////////////

namespace rotate {
  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          barrier_(parameters.Get(0)),
          vector_(parameters.Get(0)) {
    }

    // One-shot
    void Run() {
      twist::test_utils::ScopedExecutor executor;
      size_t threads = parameters_.Get(0);
      for (size_t t = 0; t < threads; ++t) {
        executor.Submit(&Tester::RunTestThread, this, t);
      }
    }

   private:
    void RunTestThread(size_t thread_index) {
      vector_[thread_index] = thread_index;

      barrier_.Arrive();

      size_t threads = parameters_.Get(0);
      size_t iterations = parameters_.Get(1);

      for (size_t i = 0; i < iterations; ++i) {
        // Choose slot to move
        size_t slot = (thread_index + i) % threads;

        // Move value to previous slot
        auto value = vector_[slot];
        barrier_.Arrive();
        vector_[Prev(slot)] = value;
        barrier_.Arrive();
      }

      ASSERT_EQ(
        vector_[thread_index],
        (thread_index + iterations) % threads);
    }

    size_t Prev(size_t slot) const {
      return (slot > 0) ? (slot - 1) : parameters_.Get(0) - 1;
    }

   private:
    TTestParameters parameters_;
    solutions::CyclicBarrier barrier_;
    std::vector<size_t> vector_;
  };
}

void RotateVectorStressTest(TTestParameters parameters) {
  rotate::Tester(parameters).Run();
}

T_TEST_CASES(RotateVectorStressTest)
    .TimeLimit(std::chrono::seconds(45))
    .Case({2, 50001})
    .Case({5, 50007})
    .Case({10, 25011})
    .Case({15, 10007});

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
