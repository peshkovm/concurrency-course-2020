#include "ticket_lock.hpp"

#include <twist/support/random.hpp>

#include <twist/fault/adversary/adversary.hpp>
#include <twist/fault/adversary/inject_fault.hpp>

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/executor.hpp>

#include <twist/strand/spin_wait.hpp>

#include <vector>
#include <chrono>

////////////////////////////////////////////////////////////////////////////////

namespace stress {

  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          start_barrier_(parameters.Get(0) + parameters.Get(1)) {
    }

    // One-shot
    void Run() {
      {
        twist::test_utils::ScopedExecutor executor;
        for (size_t t = 0; t < parameters_.Get(0); ++t) {
          executor.Submit(&Tester::RunTryLockThread, this);
        }
        for (size_t t = 0; t < parameters_.Get(1); ++t) {
          executor.Submit(&Tester::RunLockThread, this);
        }
      }
    }

   private:
    void RunLockThread() {
      start_barrier_.PassThrough();

      for (size_t i = 0; i < parameters_.Get(2); ++i) {
        spinlock_.Lock();
        CriticalSection();
        spinlock_.Unlock();
      }
    }

    void RunTryLockThread() {
      start_barrier_.PassThrough();

      for (size_t i = 0; i < parameters_.Get(2); ++i) {

        twist::strand::SpinWait spin_wait;
        while (!spinlock_.TryLock()) {
          spin_wait();
        }
        CriticalSection();
        spinlock_.Unlock();
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
    solutions::TicketLock spinlock_;
  };

};

void TryLockStressTest(TTestParameters parameters) {
  stress::Tester(parameters).Run();
}

// Parameters: TryLock threads, Lock threads, iterations

T_TEST_CASES(TryLockStressTest)
  .TimeLimit(std::chrono::seconds(30))
  .Case({2, 2, 10000})
  .Case({5, 5, 10000});


#if defined(TWIST_FIBER)

// Extra test cases
T_TEST_CASES(TryLockStressTest)
  .TimeLimit(std::chrono::minutes(1))
  .Case({10, 10, 100000});

#endif

////////////////////////////////////////////////////////////////////////////////

namespace philosophers {
  using Fork = solutions::TicketLock;

  class Plate {
   public:
    void Access() {
      ASSERT_FALSE(accessed_.exchange(true, std::memory_order_relaxed));
      twist::fault::InjectFault();
      ASSERT_TRUE(accessed_.exchange(false, std::memory_order_relaxed));
    }

   private:
    std::atomic<bool> accessed_{false};
  };

  class Table {
   public:
    Table(size_t num_seats)
        : num_seats_(num_seats),
          plates_(num_seats_),
          forks_(num_seats_) {
    }

    Fork& LeftFork(size_t seat) {
      return forks_[seat];
    }

    Fork& RightFork(size_t seat) {
      return forks_[ToRight(seat)];
    }

    size_t ToRight(size_t seat) const {
      return (seat + 1) % num_seats_;
    }

    void AccessPlate(size_t seat) {
      plates_[seat].Access();
    }

   private:
    size_t num_seats_;
    std::vector<Plate> plates_;
    std::vector<Fork> forks_;
  };

  class Philosopher {
   public:
    Philosopher(Table& table, size_t seat)
        : table_(table),
          seat_(seat),
          left_fork_(table_.LeftFork(seat_)),
          right_fork_(table_.RightFork(seat_)) {
    }

    void EatOneMoreTime() {
      AcquireForks();
      Eat();
      ReleaseForks();
      Think();
    }

   private:
    void AcquireForks() {
      while (true) {
        left_fork_.Lock();
        if (right_fork_.TryLock()) {
          ASSERT_FALSE(right_fork_.TryLock());
          return;
        } else {
          left_fork_.Unlock();
        }
      }
    }

    void Eat() {
      table_.AccessPlate(seat_);
      table_.AccessPlate(table_.ToRight(seat_));
    }

    void ReleaseForks() {
      if (twist::TossFairCoin()) {
        ReleaseForks(left_fork_, right_fork_);
      } else {
        ReleaseForks(right_fork_, left_fork_);
      }
    };

    static void ReleaseForks(Fork& p, Fork& q) {
      p.Unlock();
      q.Unlock();
    }

    void Think() {
      twist::fault::InjectFault();
    }

   private:
    Table& table_;
    size_t seat_;

    Fork& left_fork_;
    Fork& right_fork_;
  };

  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          start_barrier_(parameters_.Get(0)),
          table_(parameters_.Get(1)) {
    }

    // One-shot method
    void Run() {
      twist::test_utils::ScopedExecutor executor;
      size_t threads = parameters_.Get(0);
      for (size_t i = 0; i < threads; ++i) {
        executor.Submit(&Tester::RunTestThread, this, i);
      }
    }

   private:
    void RunTestThread(size_t thread_index) {
      Philosopher philosopher(table_, thread_index);

      start_barrier_.PassThrough();

      size_t iterations = parameters_.Get(1);
      for (size_t i = 0; i < iterations; ++i) {
        philosopher.EatOneMoreTime();
      }
    }

   private:
    TTestParameters parameters_;
    twist::test_utils::OnePassBarrier start_barrier_;
    Table table_;
  };
}  // namespace philosophers

void PhilosophersStressTest(TTestParameters parameters) {
  philosophers::Tester(parameters).Run();
}


// Parameters: Threads (philosophers), iterations

T_TEST_CASES(PhilosophersStressTest)
  .TimeLimit(std::chrono::seconds(30))
  .Case({2, 1000})
  .Case({3, 50000})
  .Case({5, 50000})
  .Case({10, 10000});

#if defined(TWIST_FIBER)

// Extra test cases
T_TEST_CASES(PhilosophersStressTest)
  .TimeLimit(std::chrono::seconds(10))
  .Case({5, 100000})
  .Case({50, 10000});

#endif

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
