#include <condvar.hpp>

#include <twist/support/random.hpp>

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/executor.hpp>

#include <twist/stdlike/mutex.hpp>

#include <deque>

////////////////////////////////////////////////////////////////////////////////

namespace robot {
  void NotifyAtLeastOneThread(solutions::ConditionVariable& condvar) {
    if (twist::TossFairCoin()) {
      condvar.NotifyOne();
    } else {
      condvar.NotifyAll();
    }
  }

  class Tester {
    enum class Step {Left, Right};

   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters) {
    }

    // One-shot
    void Run() {
      twist::test_utils::ScopedExecutor executor;
      executor.Submit(&Tester::RightFoot, this);
      executor.Submit(&Tester::LeftFoot, this);
    }

   private:
    void LeftFoot() {
      for (size_t i = 0; i < parameters_.Get(0); ++i) {
        MakeLeftStep();
      }
    }

    void MakeLeftStep() {
      std::unique_lock lock(mutex_);
      while (step_ != Step::Left) {
        switched_.Wait(lock);
      }
      ASSERT_TRUE(step_ == Step::Left);
      step_ = Step::Right;
      lock.unlock();
      NotifyAtLeastOneThread(switched_);
    }

    void RightFoot() {
      for (size_t i = 0; i < parameters_.Get(0); ++i) {
        MakeRightStep();
      }
    }

    void MakeRightStep() {
      std::unique_lock lock(mutex_);
      while (step_ != Step::Right) {
        switched_.Wait(lock);
      }
      ASSERT_TRUE(step_ == Step::Right);
      step_ = Step::Left;
      NotifyAtLeastOneThread(switched_);
    }

   private:
    TTestParameters parameters_;

    Step step_{Step::Left};
    twist::stdlike::mutex mutex_;
    solutions::ConditionVariable switched_;
  };
}

void RobotStepsStressTest(TTestParameters parameters) {
  robot::Tester(parameters).Run();
}

T_TEST_CASES(RobotStepsStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({10000})
    .Case({100000});

#if defined(TWIST_FIBER)

T_TEST_CASES(RobotStepsStressTest)
    .TimeLimit(std::chrono::seconds(10))
    .Case({1000000});

#endif

////////////////////////////////////////////////////////////////////////////////

namespace queue {
  template <typename T>
  class UnboundedBlockingQueue {
   public:
    void Enqueue(T item) {
      std::unique_lock lock(mutex_);
      items_.push_back(std::move(item));

      if (twist::TossFairCoin()) {
        lock.unlock();
      }
      not_empty_.NotifyOne();
    }

    T Dequeue() {
      std::unique_lock lock(mutex_);
      while (items_.empty()) {
        not_empty_.Wait(lock);
      }
      auto item = items_.front();
      items_.pop_front();
      return item;
    }

   private:
    std::deque<int> items_;
    twist::stdlike::mutex mutex_;
    solutions::ConditionVariable not_empty_;
  };

  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          start_barrier_(parameters.Get(0) + parameters.Get(1)),
          producers_left_(parameters.Get(0)) {
    }

    // One-shot
    void Run() {
      twist::test_utils::ScopedExecutor executor;

      size_t producers = parameters_.Get(0);
      for (size_t t = 0; t < producers; ++t) {
        executor.Submit(&Tester::RunProducerThread, this, t);
      }
      size_t consumers = parameters_.Get(1);
      for (size_t t = 0; t < consumers; ++t) {
        executor.Submit(&Tester::RunConsumerThread, this);
      }
      executor.Join();

      ASSERT_EQ(total_consumed_.load(), total_produced_.load());
    }

   private:
    void RunProducerThread(size_t thread_index) {
      start_barrier_.PassThrough();

      size_t producers = parameters_.Get(0);
      size_t range = parameters_.Get(2);

      for (size_t i = 0; i < range; ++i) {
        if (i % producers == thread_index) {
          queue_.Enqueue((int)i);
          total_produced_.fetch_add((int)i);
        }
      }

      size_t consumers = parameters_.Get(1);

      if (producers_left_.fetch_sub(1) == 1) {
        // last producer
        for (size_t i = 0; i < consumers; ++i) {
          queue_.Enqueue(-1); // put poison pill
        }
      }
    }

    void RunConsumerThread() {
      start_barrier_.PassThrough();

      while (true) {
        int value = queue_.Dequeue();
        if (value == -1) {
           break; // poison pill
        }
        total_consumed_.fetch_add(value);
      }
    }

   private:
    TTestParameters parameters_;

    twist::test_utils::OnePassBarrier start_barrier_;

    UnboundedBlockingQueue<int> queue_;

    std::atomic<int> total_consumed_{0};
    std::atomic<int> total_produced_{0};

    std::atomic<size_t> producers_left_;
  };
}

void QueueStressTest(TTestParameters parameters) {
  queue::Tester(parameters).Run();
}

T_TEST_CASES(QueueStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({5, 1, 10000})
    .Case({1, 5, 10000})
    .Case({5, 5, 10000})
    .Case({10, 10, 10000});

RUN_ALL_TESTS()
