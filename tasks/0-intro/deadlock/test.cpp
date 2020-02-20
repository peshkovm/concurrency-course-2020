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
      mutex.Lock();
      mutex.Lock();
      // use mutex.Lock() / mutex.Unlock() to lock/unlock mutex
    };

    // RunScheduler(fiber) should throw DeadlockDetected exception
    ASSERT_THROW(RunScheduler(fiber), DeadlockDetected);
  }

  // Deadlock with two fibers
  SIMPLE_TEST(TwoFibers) {
    Mutex a, b;

    auto finn = [&]() {
      a.Lock();
      Yield();
      b.Lock();
      b.Unlock();
      a.Unlock();
      Yield();
      // Use Yield() to reschedule current fiber
    };

    auto jake = [&]() {
      b.Lock();
      Yield();
      a.Lock();
      a.Unlock();
      b.Unlock();
    };

    // Don't change this routine
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
