#include <twist/test_framework/test_framework.hpp>

#include <twist/fiber/core/api.hpp>
#include <twist/fiber/sync/mutex.hpp>

using twist::fiber::DeadlockDetected;
using twist::fiber::Mutex;
using twist::fiber::RunScheduler;
using twist::fiber::Spawn;
using twist::fiber::Yield;

TEST_SUITE(Deadlock) {
  // Deadlock with one fiber and one mutex
  SIMPLE_TEST(OneFiber) {
    Mutex mutex;

    auto fiber = [&]() {
      // Your code goes here
      // use mutex.Lock() / mutex.Unlock() to lock/unlock mutex
      mutex.Lock();
      mutex.Lock();
    };

    // RunScheduler(fiber) should throw DeadlockDetected exception
    ASSERT_THROW(RunScheduler(fiber), DeadlockDetected);
  }

  // Deadlock with two fibers
  SIMPLE_TEST(TwoFibers) {
    // Declare some Mutex-es
    Mutex mutex_finn, mutex_jake;

    auto finn = [&]() {
      // Your code goes here
      // Use Yield() to reschedule current fiber
      mutex_finn.Lock();
      Yield();
      mutex_jake.Lock();
      mutex_jake.Unlock();
      mutex_finn.Unlock();
    };

    auto jake = [&]() {
      // Your code goes here
      mutex_jake.Lock();
      Yield();
      mutex_finn.Lock();
      mutex_finn.Unlock();
      mutex_jake.Unlock();
    };

    // Don't change this routine910033
    auto adventure = [&]() {
      // Run two cooperative fibers
      Spawn(finn);
      Spawn(jake);
    };

    // Don't block separately
    RunScheduler(finn);
    RunScheduler(finn);

    RunScheduler(jake);
    RunScheduler(jake);

    // Finn and Jake should block each other
    ASSERT_THROW(RunScheduler(adventure), DeadlockDetected);
  }
}

RUN_ALL_TESTS()
