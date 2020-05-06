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

////////////////////////////////////////////////////////////////////////////////

void MissedWakeupInJoin(TTestParameters parameters) {
  WallTimeBudget wall_time_budget(
      std::chrono::seconds(parameters.Get(0)));

  while (!wall_time_budget.Exhausted()) {
    auto tp = MakeStaticThreadPool(1, "test");

    std::atomic<bool> task_done{false};

    tp->Execute([&task_done](){
      task_done.store(true);
    });

    while (!task_done.load()) {};
    tp->Join();
  }
}

T_TEST_CASES(MissedWakeupInJoin).TimeLimit(15s).Case({10});

////////////////////////////////////////////////////////////////////////////////
