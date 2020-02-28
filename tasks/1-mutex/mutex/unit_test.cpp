#include "mutex.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/executor.hpp>
#include <twist/test_utils/mutex_tester.hpp>
#include <twist/test_utils/cpu_timer.hpp>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

TEST_SUITE(UnitTest) {
  SIMPLE_T_TEST(LockUnlock) {
    solutions::Mutex mutex;
    mutex.Lock();
    mutex.Unlock();
  }

  SIMPLE_T_TEST(SequentialLockUnlock) {
    solutions::Mutex mutex;
    mutex.Lock();
    mutex.Unlock();
    mutex.Lock();
    mutex.Unlock();
  }

  SIMPLE_T_TEST(SharedMemoryValue) {
    // Тестируем использование общей ячейки памяти на разные инстансы локов
    solutions::Mutex mutex;
    mutex.Lock();

    solutions::Mutex mutex2;
    mutex2.Lock();  // тут не должно блокироваться
    mutex2.Unlock();

    mutex.Unlock();
  }

  SIMPLE_T_TEST(ConcurrentLock) {
    twist::test_utils::MutexTester<solutions::Mutex> mutex;

    volatile int counter = 0;

    auto routine = [&mutex, &counter]() {
      mutex.Lock();
      twist::strand::this_thread::sleep_for(100ms);
      counter++;
      mutex.Unlock();
    };

    twist::test_utils::ScopedExecutor executor;
    executor.Submit(routine);
    executor.Submit(routine);
    executor.Join();

    ASSERT_EQ(2, counter);
  }

#if !defined(TWIST_FIBER)

  SIMPLE_T_TEST(Blocking) {
    solutions::Mutex mutex;

    twist::strand::thread sleeper([&]() {
      mutex.Lock();
      twist::strand::this_thread::sleep_for(3s);
      mutex.Unlock();
    });

    twist::strand::thread waiter([&]() {
      twist::strand::this_thread::sleep_for(1s);
      CPUTimer timer;
      mutex.Lock();
      mutex.Unlock();
      double running_time = timer.RunningTime();
      std::cout << "Lock/Unlock cpu time in sleeper thread: " << running_time << " seconds\n";
      ASSERT_TRUE(running_time < 0.1);
    });

    sleeper.join();
    waiter.join();
  }

  SIMPLE_T_TEST(NoContentionNoFutexes) {
    static const size_t kNoContentionIterations = 1000;

    size_t futex_calls = twist::thread::FutexCallCount();

    solutions::Mutex mutex;
    for (size_t i = 0; i < kNoContentionIterations; ++i) {
      mutex.Lock();
      mutex.Unlock();
    }

    // no calls to futex
    ASSERT_EQ(futex_calls, twist::thread::FutexCallCount());
  }

  SIMPLE_T_TEST(NoContentionNoFutexesWithWarmup) {
    solutions::Mutex mutex;

    static const size_t kWarmupIterations = 100000;

    auto contender = [&]() {
      for (size_t i = 0; i < kWarmupIterations; ++i) {
        mutex.Lock();
        mutex.Unlock();
      }
    };

    twist::strand::thread t1(contender);
    twist::strand::thread t2(contender);
    t1.join();
    t2.join();

    size_t futex_calls = twist::thread::FutexCallCount();

    const size_t kNoContentionIterations = 1000;

    for (size_t i = 0; i < kNoContentionIterations; ++i) {
      mutex.Lock();
      mutex.Unlock();
    }

    ASSERT_EQ(futex_calls, twist::thread::FutexCallCount());
  }

#endif

}

RUN_ALL_TESTS()
