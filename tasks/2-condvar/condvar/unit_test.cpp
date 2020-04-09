#include "condvar.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/strand/stdlike.hpp>

#include <atomic>
#include <chrono>

using namespace std::chrono_literals;

TEST_SUITE(CondVar) {
  SIMPLE_T_TEST(NotifyOne) {
    twist::strand::mutex mutex;
    solutions::ConditionVariable condvar;

    bool pass{false};

    auto wait_routine = [&]() {
      std::unique_lock lock(mutex);

      while (!pass) {
        condvar.Wait(lock);
      }
    };

    twist::strand::thread t(wait_routine);

    twist::strand::this_thread::sleep_for(
        std::chrono::milliseconds(250));

    {
      std::unique_lock lock(mutex);
      pass = true;
      condvar.NotifyOne();
    }

    t.join();
  }

  SIMPLE_T_TEST(BlockingWait) {
    int state = 0;
    twist::strand::mutex mutex;
    solutions::ConditionVariable condvar;

    condvar.NotifyOne();
    condvar.NotifyAll();

    auto wait_routine = [&]() {
      std::unique_lock lock(mutex);
      ASSERT_EQ(state, 0);
      condvar.Wait(mutex);
      ASSERT_EQ(state, 1);
    };

    twist::strand::thread t(wait_routine);

    twist::strand::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(state, 0);

    {
      std::unique_lock lock(mutex);
      state = 1;
      condvar.NotifyOne();
    }

    t.join();
  }

  SIMPLE_T_TEST(NotifyAll) {
    twist::strand::mutex mutex;
    solutions::ConditionVariable condvar;
    bool pass{false};

    auto wait_routine = [&]() {
      std::unique_lock lock(mutex);

      while (!pass) {
        condvar.Wait(lock);
      }
    };

    twist::strand::thread t1(wait_routine);
    twist::strand::thread t2(wait_routine);

    twist::strand::this_thread::sleep_for(
        std::chrono::milliseconds(250));

    {
      std::unique_lock lock(mutex);
      pass = true;
      condvar.NotifyAll();
    }

    t1.join();
    t2.join();
  }

  SIMPLE_T_TEST(NotifyManyTimes) {
    static const size_t kIterations = 1000 * 1000;

    solutions::ConditionVariable condvar;
    for (size_t i = 0; i < kIterations; ++i) {
      condvar.NotifyOne();
    }
  }
}

RUN_ALL_TESTS()
