#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>

#include <twist/strand/stdlike.hpp>

#include "helpers.hpp"

#include <atomic>

using namespace tiny::executors;
using namespace test_helpers;
using namespace std::chrono_literals;

namespace thread_pool {

void MissedWakeupInJoin(TTestParameters parameters) {
  WallTimeBudget wall_time_budget(
      std::chrono::seconds(parameters.Get(0)));

  while (!wall_time_budget.Exhausted()) {
    auto tp = MakeStaticThreadPool(1, "test");

    std::atomic<bool> task_done{false};

    tp->Execute([&task_done](){
      task_done.store(true);
    });

    WallTimeLimitGuard guard(1s);
    while (!task_done.load()) {};
    tp->Join();
  }
}

T_TEST(MissedWakeupInJoin).TimeLimit(15s).Case({10});

}  // namespace thread_pool

namespace strand {

////////////////////////////////////////////////////////////////////////////////

class Counter {
 public:
  Counter(IExecutorPtr e)
      : strand_(MakeStrand(e)) {
  }

  void Increment() {
    strand_->Execute([this]() {
      ++value_;
    });
  }

  size_t Value() const {
    return value_;
  }

 private:
  size_t value_{0};
  IExecutorPtr strand_;
};

void ConcurrentStrands(TTestParameters parameters) {
  size_t strands = parameters.Get(0);
  size_t iterations = parameters.Get(1);
  size_t batch_size = parameters.Get(2);

  auto tp = MakeStaticThreadPool(16, "pool");

  std::vector<Counter> counters;
  counters.reserve(strands);
  for (size_t i = 0; i < strands; ++i) {
    counters.emplace_back(tp);
  }

  for (size_t i = 0; i < iterations; ++i) {
    for (size_t j = 0; j < strands; ++j) {
      for (size_t k = 0; k < batch_size; ++k) {
        counters[j].Increment();
      }
    }
  }

  tp->Join();

  for (size_t i = 0; i < strands; ++i) {
    ASSERT_EQ(counters[i].Value(), batch_size * iterations);
  }
}

T_TEST(ConcurrentStrands)
    .TimeLimit(30s)
    .Case({50, 50, 50})
    .Case({100, 100, 20});

////////////////////////////////////////////////////////////////////////////////

void HangingStrand(TTestParameters parameters) {
  WallTimeBudget wall_time_budget(
      std::chrono::seconds(parameters.Get(0)));

  auto tp = MakeStaticThreadPool(3, "test");
  auto strand = MakeStrand(tp);

  for (size_t i = 0; !wall_time_budget.Exhausted(); ++i) {
    std::atomic<size_t> completed{0};

    auto task = [&completed]() {
      completed.store(completed.load() + 1);
    };

    size_t tasks = 2 + i % 5;

    for (size_t t = 0; t < tasks; ++t) {
      strand->Execute(task);
    }

    WallTimeLimitGuard guard(1s);
    while (completed != tasks) {
      twist::strand::this_thread::yield();
    }
  }
}

T_TEST(HangingStrand).TimeLimit(15s).Case({10});

////////////////////////////////////////////////////////////////////////////////

}  // namespace strand

RUN_ALL_TESTS()
