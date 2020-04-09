#include "semaphore.hpp"
#include "channel.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/count_down_latch.hpp>
#include <twist/test_utils/executor.hpp>
#include <twist/test_utils/mutex_tester.hpp>

#include <twist/fault/adversary/inject_fault.hpp>

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

class ResourcePool {
 public:
  ResourcePool(size_t limit)
      : limit_(limit)
      , available_(limit) {
  }

  void Access() {
    ASSERT_TRUE_M(available_.fetch_sub(1) > 0, "Resource pool exhausted");
    twist::fault::InjectFault();
    ASSERT_TRUE(available_.fetch_add(1) < (int)limit_);
  }

 private:
  size_t limit_;
  std::atomic<int> available_;
};

void PoolStressTest(const TTestParameters& parameters) {
  size_t threads = parameters.Get(0);
  twist::test_utils::OnePassBarrier start_barrier{threads};

  size_t pool_limit = parameters.Get(1);
  solutions::Semaphore semaphore{pool_limit};
  ResourcePool resource_pool{pool_limit};

  auto test_routine = [&]() {
    start_barrier.PassThrough();

    size_t iterations = parameters.Get(2);
    for (size_t i = 0; i < iterations; ++i) {
      semaphore.Acquire();
      resource_pool.Access();
      semaphore.Release();
    }
  };

  twist::test_utils::ScopedExecutor executor;
  for (size_t t = 0; t < threads; ++t) {
    executor.Submit(test_routine);
  }
}

// Parameters: threads, limit, iterations
T_TEST_CASES(PoolStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({2, 1, 50000})
    .Case({5, 3, 50000})
    .Case({10, 1, 20000})
    .Case({10, 5, 10000})
    .Case({10, 9, 10000});

////////////////////////////////////////////////////////////////////////////////

void LostWakeupStressTest(const TTestParameters& parameters) {
  size_t repeats = parameters.Get(0);
  for (size_t i = 0; i < repeats; ++i) {
    solutions::Semaphore semaphore{0};

    twist::test_utils::CountDownLatch consumers_latch{2};
    twist::test_utils::OnePassBarrier producers_barrier{2};

    auto consumer = [&semaphore, &consumers_latch]() {
      consumers_latch.CountDown();
      semaphore.Acquire();
    };

    auto producer = [&semaphore, &producers_barrier]() {
      producers_barrier.PassThrough();
      semaphore.Release();
    };

    twist::test_utils::ScopedExecutor executor;

    executor.Submit(consumer);
    executor.Submit(consumer);

    consumers_latch.Await(); // Better than nothing

    executor.Submit(producer);
    executor.Submit(producer);

    executor.Join();
  }
}

// Parameters: iterations
T_TEST_CASES(LostWakeupStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({1000});

////////////////////////////////////////////////////////////////////////////////

void PingPongStressTest(const TTestParameters& parameters) {
  solutions::Semaphore left{1};
  solutions::Semaphore right{0};

  twist::test_utils::ScopedExecutor executor;

  size_t iterations = parameters.Get(0);

  auto right_routine = [&]() {
    for (size_t i = 0; i < iterations; ++i) {
      right.Acquire();
      left.Release();
    }
  };

  executor.Submit(right_routine);

  for (size_t i = 0; i < iterations; ++i) {
    left.Acquire();
    right.Release();
  }

  left.Acquire();
}

// Parameters: iterations
T_TEST_CASES(PingPongStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({10000});

////////////////////////////////////////////////////////////////////////////////

namespace channel {

class Tester {
 public:
  Tester(const TTestParameters& parameters)
      : parameters_(parameters),
        channel_(parameters_.Get(3)),
        start_barrier_(parameters.Get(0) + parameters.Get(1)),
        producer_count_(parameters_.Get(0)) {
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

    ASSERT_EQ(total_produced_.load(), total_consumed_.load());
  }

 private:
  void RunProducerThread(size_t thread_index) {
    start_barrier_.PassThrough();

    size_t items = parameters_.Get(2);
    size_t producers = parameters_.Get(0);
    for (size_t i = thread_index; i < items; i += producers) {
      channel_.Send(std::to_string(i));
      total_produced_.fetch_add(i);
    }

    if (producer_count_.fetch_sub(1) == 1) {  // Last producer
      channel_.Send("");  // Poison pill
    }
  }

  void RunConsumerThread() {
    start_barrier_.PassThrough();

    while (true) {
      std::string item = channel_.Receive();
      if (item.empty()) {
        break;
      }
      int value = std::stoi(item);
      total_consumed_.fetch_add(value);
    }
    channel_.Send("");  // Poison pill
  }

 private:
  TTestParameters parameters_;
  solutions::BufferedChannel<std::string> channel_;
  twist::test_utils::OnePassBarrier start_barrier_;
  std::atomic<size_t> producer_count_{0};
  std::atomic<size_t> total_produced_{0};
  std::atomic<size_t> total_consumed_{0};
};

}

void ChannelStressTest(const TTestParameters& parameters) {
  channel::Tester(parameters).Run();
}

// Parameters: producers, consumers, items, channel capacity
T_TEST_CASES(ChannelStressTest)
    .TimeLimit(std::chrono::seconds(30))
    .Case({1, 1, 100000, 1})
    .Case({5, 5, 100000, 100})
    .Case({7, 2, 100000, 3})
    .Case({3, 9, 100000, 4});

////////////////////////////////////////////////////////////////////////////////

RUN_ALL_TESTS()
