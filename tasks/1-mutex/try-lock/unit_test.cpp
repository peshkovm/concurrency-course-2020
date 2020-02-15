#include "ticket_lock.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <thread>

TEST_SUITE(TicketTryLock) {
  SIMPLE_T_TEST(LockUnlock) {
    solutions::TicketLock spinlock;

    spinlock.Lock();
    spinlock.Unlock();
  }

  SIMPLE_T_TEST(SequentialLockUnlock) {
    solutions::TicketLock spinlock;

    spinlock.Lock();
    spinlock.Unlock();
    spinlock.Lock();
    spinlock.Unlock();
  }

  SIMPLE_T_TEST(TryLock) {
    solutions::TicketLock spinlock;

    ASSERT_TRUE(spinlock.TryLock());
    ASSERT_FALSE(spinlock.TryLock());
    spinlock.Unlock();
    ASSERT_TRUE(spinlock.TryLock());
    spinlock.Unlock();
    spinlock.Lock();
    ASSERT_FALSE(spinlock.TryLock());
  }
}

RUN_ALL_TESTS()
