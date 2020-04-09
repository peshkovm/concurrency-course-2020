#include "cyclic_barrier.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/strand/stdlike.hpp>

TEST_SUITE(CyclicBarrier) {
  SIMPLE_T_TEST(OneThread) {
    solutions::CyclicBarrier barrier{1};

    for (size_t i = 0; i < 10; ++i) {
      barrier.Arrive();
    }
  }

  SIMPLE_T_TEST(TwoThreads) {
    solutions::CyclicBarrier barrier{2};

    int my = 0;
    int that = 0;

    auto that_routine = [&]() {
      that = 1;
      barrier.Arrive();
      ASSERT_EQ(my, 1);
      barrier.Arrive();
      that = 2;
      barrier.Arrive();
      ASSERT_EQ(my, 2);
    };

    twist::strand::thread that_thread(that_routine);

    my = 1;
    barrier.Arrive();
    ASSERT_EQ(that, 1);
    barrier.Arrive();
    my = 2;
    barrier.Arrive();
    ASSERT_EQ(that, 2);

    that_thread.join();
  }

  SIMPLE_T_TEST(Runners) {
    static const size_t kThreads = 10;
    solutions::CyclicBarrier barrier{kThreads};

    static const size_t kIterations = 256;

    auto runner_routine = [&barrier]() {
      for (size_t i = 0; i < kIterations; ++i) {
        barrier.Arrive();
      }
    };

    std::vector<twist::strand::thread> runners;

    for (size_t i = 0; i < kThreads; ++i) {
      runners.emplace_back(runner_routine);
    }

    for (auto& runner : runners) {
      runner.join();
    }
  }
}

RUN_ALL_TESTS()
