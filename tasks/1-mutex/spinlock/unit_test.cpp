#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include "spinlock.hpp"

TEST_SUITE(SpinLock) {
  SIMPLE_T_TEST(LockUnlock) {
    solutions::SpinLock spinlock;
    spinlock.Lock();
    spinlock.Unlock();
  }

  SIMPLE_T_TEST(SequentialLockUnlock) {
    solutions::SpinLock spinlock;
    spinlock.Lock();
    spinlock.Unlock();
    spinlock.Lock();
    spinlock.Unlock();
  }

  SIMPLE_T_TEST(TryLock) {
    solutions::SpinLock spinlock;
    ASSERT_TRUE(spinlock.TryLock());
    ASSERT_FALSE(spinlock.TryLock());
    spinlock.Unlock();
    ASSERT_TRUE(spinlock.TryLock());
    spinlock.Unlock();
    spinlock.Lock();
    ASSERT_FALSE(spinlock.TryLock());
  }

  SIMPLE_T_TEST(Exchange) {
    std::int64_t var = 0;
    for (std::int64_t i = 0; i < 10; ++i) {
      ASSERT_EQ(i, AtomicExchange(&var, i + 1));
    }
  }
}

RUN_ALL_TESTS()
