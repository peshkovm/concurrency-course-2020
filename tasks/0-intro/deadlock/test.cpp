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
    };

    // RunScheduler(fiber) should throw DeadlockDetected exception
    ASSERT_THROW(RunScheduler(fiber), DeadlockDetected);
  }

  // Deadlock with two fibers
  SIMPLE_TEST(TwoFibers) {
    // Declare some Mutex-es

    auto finn = [&]() {
      // Your code goes here
      // Use Yield() to reschedule current fiber
    };

    auto jake = [&]() {
      // Your code goes here
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
