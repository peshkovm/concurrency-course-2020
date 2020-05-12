#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>
#include <tinyfutures/futures/promise.hpp>

#include <twist/strand/stdlike.hpp>

#include <atomic>

using namespace tiny::executors;

using tiny::futures::Promise;
using tiny::futures::Future;
using tiny::futures::MakeContract;

using tiny::support::Result;
using tiny::support::Unit;

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////

void Pipelines(TTestParameters parameters) {
  size_t pipelines = parameters.Get(0);

  static const size_t kThreads = 4;
  auto tp = MakeStaticThreadPool(kThreads, "tp");
  auto strand = MakeStrand(tp);

  for (size_t i = 0; i < kThreads; ++i) {
    tp->Execute([]() {
      twist::strand::this_thread::sleep_for(100ms);
    });
  }

  std::atomic<size_t> total = 0;
  auto bump_total = [&total](Result<Unit>) {
    total.fetch_add(1);
    return Unit{};
  };

  size_t total_strand = 0;
  auto bump_total_strand = [&total_strand](Result<Unit>) {
    ++total_strand;
    return Unit{};
  };

  for (size_t i = 0; i < pipelines; ++i) {
    auto [f, p] = MakeContract<Unit>();
    std::move(f).Via(tp).Then(bump_total).Via(strand).Then(bump_total_strand);
    std::move(p).SetValue({});
  }

  tp->Join();

  ASSERT_EQ(total.load(), pipelines);
  ASSERT_EQ(total_strand, pipelines);
}

T_TEST_CASES(Pipelines).TimeLimit(30s).Case({12345});

#if defined(TWIST_FIBER)
T_TEST_CASES(Pipelines).TimeLimit(30s).Case({1234567});
#endif

////////////////////////////////////////////////////////////////////////////////

void RacesSetGet(TTestParameters parameters) {
  size_t collisions = parameters.Get(0);

  auto get_tp = MakeStaticThreadPool(8, "set");
  auto set_tp = MakeStaticThreadPool(8, "get");

  for (int i = 0; i < (int)collisions; ++i) {
    auto [f, p] = MakeContract<int>();

    set_tp->Execute([i, p = std::move(p)]() mutable {
      std::move(p).SetValue(i);
    });
    get_tp->Execute([i, f = std::move(f)]() mutable {
      ASSERT_EQ(std::move(f).GetValue(), i);
    });
  }

  set_tp->Join();
  get_tp->Join();

  ASSERT_EQ(get_tp->ExecutedTaskCount(), collisions);
}

T_TEST_CASES(RacesSetGet)
    .TimeLimit(30s)
    .Case({10000})
    .Case({50000});

#if defined(TWIST_FIBER)
T_TEST_CASES(RacesSetGet).TimeLimit(30s).Case({1000000});
#endif

////////////////////////////////////////////////////////////////////////////////

void RacesSetSubscribe(TTestParameters parameters) {
  size_t collisions = parameters.Get(0);

  auto sub_tp = MakeStaticThreadPool(8, "set");
  auto set_tp = MakeStaticThreadPool(8, "get");

  std::atomic<size_t> sub_done{0};

  for (int i = 0; i < (int)collisions; ++i) {
    auto [f, p] = MakeContract<int>();

    set_tp->Execute([i, p = std::move(p)]() mutable {
      std::move(p).SetValue(i);
    });
    sub_tp->Execute([i, f = std::move(f), &sub_done]() mutable {
      std::move(f).Subscribe([i, &sub_done](Result<int> result) {
        ASSERT_EQ(i, result.Value());
        sub_done.fetch_add(1);
      });
    });
  }

  set_tp->Join();
  sub_tp->Join();

  ASSERT_EQ(sub_tp->ExecutedTaskCount(), collisions);
  ASSERT_EQ(sub_done.load(), collisions);
}

T_TEST_CASES(RacesSetSubscribe)
    .TimeLimit(30s)
    .Case({10000})
    .Case({50000});

#if defined(TWIST_FIBER)
T_TEST_CASES(RacesSetSubscribe).TimeLimit(30s).Case({1000000});
#endif

////////////////////////////////////////////////////////////////////////////////
