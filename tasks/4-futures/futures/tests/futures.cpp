#include <twist/test_framework/test_framework.hpp>

#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>
#include <tinyfutures/executors/work.hpp>
#include <tinyfutures/executors/thread_label.hpp>

#include <tinyfutures/futures/promise.hpp>
#include <tinyfutures/futures/after.hpp>
#include <tinyfutures/futures/async.hpp>
#include <tinyfutures/futures/combine.hpp>
#include <tinyfutures/futures/with_timeout.hpp>

#include "helpers.hpp"

#include <thread>
#include <atomic>

using namespace std::chrono_literals;

using namespace tiny::executors;
using namespace tiny::futures;

using tiny::support::Unit;
using tiny::support::make_result::Invoke;
using tiny::support::Duration;

using test_helpers::AsyncValue;
using test_helpers::AsyncError;

TEST_SUITE_WITH_PRIORITY(Futures, 2) {
  SIMPLE_TEST(JustWorks) {
    Promise<int> p;
    auto f = p.MakeFuture();

    std::move(p).SetValue(42);

    ASSERT_TRUE(f.IsValid());
    ASSERT_EQ(std::move(f).GetValue(), 42);
    ASSERT_FALSE(f.IsValid());
  }

  SIMPLE_TEST(JustWorksWithContract) {
    auto [f, p] = MakeContract<int>();

    std::move(p).SetValue(42);

    ASSERT_TRUE(f.IsValid());
    ASSERT_EQ(std::move(f).GetValue(), 42);
    ASSERT_FALSE(f.IsValid());
  }

  SIMPLE_TEST(Exception) {
    auto [f, p] = MakeContract<std::string>();

    try {
      throw std::runtime_error("test");
    } catch (...) {
      std::move(p).SetError(std::current_exception());
    }

    ASSERT_THROW(std::move(f).GetValue(), std::runtime_error);
  }

  SIMPLE_TEST(ExceptionWithGetResult) {
    auto [f, p] = MakeContract<std::string>();

    try {
      throw std::runtime_error("test");
    } catch (...) {
      std::move(p).SetError(std::current_exception());
    }

    auto result = std::move(f).GetResult();
    ASSERT_TRUE(result.HasError());
    ASSERT_FALSE(f.IsValid());
  }

  SIMPLE_TEST(BlockingGetValue) {
    static const std::string kMessage = "Hello!";

    auto tp = MakeStaticThreadPool(1, "tp");

    auto [f, p] = MakeContract<std::string>();
    tp->Execute([p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      std::move(p).SetValue(kMessage);
    });

    {
      test_helpers::CPUTimeBudgetGuard budget(0.1);
      auto message = std::move(f).GetValue();
      ASSERT_EQ(message, kMessage);
    }

    tp->Join();
  }

  SIMPLE_TEST(AfterJustWorks) {
    auto f = After(1100ms);

    test_helpers::StopWatch stop_watch;
    std::move(f).GetValue();
    ASSERT_GT(stop_watch.Elapsed(), 1s);
  }

  SIMPLE_TEST(ConcurrentAfters) {
    auto f1 = After(1100ms);
    auto f2 = After(1200ms);

    test_helpers::StopWatch stop_watch;
    std::move(f1).GetValue();
    std::move(f2).GetValue();
    ASSERT_GT(stop_watch.Elapsed(), 1s);
    ASSERT_LT(stop_watch.Elapsed(), 1500ms);
  }

  SIMPLE_TEST(AsyncVia) {
    auto tp = MakeStaticThreadPool(3, "tp");

    {
      auto good = []() -> std::string {
        ExpectThread("tp");
        return "Hello!";
      };

      auto f = AsyncVia(good, tp);
      ASSERT_EQ(std::move(f).GetValue(), "Hello!");
    }

    {
      auto bad = []() -> int {
        ExpectThread("tp");
        throw std::logic_error("test");
      };

      auto result = AsyncVia(bad, tp).GetResult();
      ASSERT_TRUE(result.HasError());
      ASSERT_THROW(result.ThrowIfError(), std::logic_error);
    }

    tp->Join();
  }

  SIMPLE_TEST(Subscribe1) {
    auto [f, p] = MakeContract<int>();

    std::move(p).SetValue(17);

    bool called = false;
    std::move(f).Subscribe([&called](Result<int> v) {
      ASSERT_EQ(v.Value(), 17);
      called = true;
    });

    ASSERT_FALSE(f.IsValid());
    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(Subscribe2) {
    auto [f, p] = MakeContract<int>();

    auto result = Invoke([]() -> int {
      throw std::runtime_error("test");
    });

    std::move(p).Set(std::move(result));

    bool called = false;
    std::move(f).Subscribe([&called](Result<int> v) {
      ASSERT_TRUE(v.HasError());
      called = true;
    });

    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(Subscribe3) {
    static const std::string kMessage = "Hello!";

    auto tp = MakeStaticThreadPool(1, "tp");

    auto [f, p] = MakeContract<std::string>();

    std::atomic<bool> called{false};

    std::move(f).Subscribe([&called, expected = kMessage](Result<std::string> v) {
      ExpectThread("tp");
      ASSERT_EQ(v.Value(), expected);
      called.store(true);
    });

    ASSERT_FALSE(f.IsValid());
    ASSERT_FALSE(called.load());

    tp->Execute([p = std::move(p), message = kMessage]() mutable {
      std::move(p).SetValue(message);
    });

    tp->Join();

    ASSERT_TRUE(called.load());
  }

  SIMPLE_TEST(SubscribeVia1) {
    test_helpers::CPUTimeBudgetGuard budget(0.1);

    auto tp = MakeStaticThreadPool(1, "callbacks");

    auto [f, p] = MakeContract<int>();

    std::move(p).SetValue(17);

    std::atomic<bool> called = false;

    auto callback = [&called](Result<int> v) mutable {
      ExpectThread("callbacks");
      ASSERT_EQ(v.Value(), 17);
      called.store(true);
    };

    // Schedule immediately
    std::move(f).Via(tp).Subscribe(callback);

    tp->Join();

    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(SubscribeVia2) {
    test_helpers::CPUTimeBudgetGuard budget(0.1);

    auto tp_callbacks = MakeStaticThreadPool(1, "callbacks");
    auto tp_work = MakeStaticThreadPool(1, "work");

    auto [f, p] = MakeContract<int>();

    std::atomic<bool> called = false;

    auto callback = [&called](Result<int> v) mutable {
      ExpectThread("callbacks");
      ASSERT_EQ(v.Value(), 42);
      called.store(true);
    };

    std::move(f).Via(KeepWorking(tp_callbacks)).Subscribe(callback);

    tp_work->Execute([p = std::move(p)]() mutable {
      ExpectThread("work");
      std::this_thread::sleep_for(1s);
      std::move(p).SetValue(42);
    });

    tp_callbacks->Join();
    tp_work->Join();

    ASSERT_TRUE(called);
  }

  SIMPLE_TEST(All) {
    std::vector<Future<int>> fs;

    fs.push_back(AsyncValue(1, 500ms));
    fs.push_back(AsyncValue(2, 1500ms));
    fs.push_back(AsyncValue(3, 1s));

    auto ints = All(std::move(fs)).GetValue();
    std::sort(ints.begin(), ints.end());

    ASSERT_EQ(ints, std::vector<int>({1, 2, 3}));
  }

  SIMPLE_TEST(AllWithErrors) {
    std::vector<Future<int>> fs;

    fs.push_back(AsyncValue(1, 500ms));
    fs.push_back(AsyncValue(2, 500ms));
    fs.push_back(AsyncError<int>(1s));

    auto result = All(std::move(fs)).GetResult();

    ASSERT_TRUE(result.HasError());
  }

  SIMPLE_TEST(FirstOf) {
    std::vector<Future<int>> fs;

    fs.push_back(AsyncValue<int>(1, 2s));
    fs.push_back(AsyncValue<int>(2, 1s));
    fs.push_back(AsyncValue<int>(3, 3s));

    ASSERT_EQ(FirstOf(std::move(fs)).GetValue(), 2);
  }

  SIMPLE_TEST(FirstOfWithErrors) {
    std::vector<Future<int>> fs;

    fs.push_back(AsyncError<int>(500ms));
    fs.push_back(AsyncValue(13, 2s));
    fs.push_back(AsyncError<int>(1500ms));
    fs.push_back(AsyncValue(42, 1s));

    auto f = FirstOf(std::move(fs));
    ASSERT_EQ(std::move(f).GetValue(), 42);
  }

  SIMPLE_TEST(WithTimeout) {
    {
      auto f = WithTimeout(AsyncValue(42, 500ms), 1s);
      ASSERT_EQ(std::move(f).GetValue(), 42);
    }

    {
      auto f = WithTimeout(AsyncValue(42, 2s), 1s);
      ASSERT_THROW(std::move(f).GetValue(), TimedOut);
    }
  }

  SIMPLE_TEST(ThenSynchronous) {
    test_helpers::CPUTimeBudgetGuard budget(0.1);

    auto [f, p] = MakeContract<int>();

    bool done = false;

    auto stage1 = [](Result<int> r) -> int {
      return r.Value() * 2;
    };
    auto stage2 = [](Result<int> r) -> int {
      return r.Value() + 1;
    };
    auto stage3 = [&done](Result<int> r) -> Unit {
      ASSERT_EQ(r.Value(), 15);
      done = true;
      return {};
    };

    std::move(f).Then(stage1).Then(stage2).Then(stage3);

    ASSERT_FALSE(f.IsValid());
    ASSERT_FALSE(done);

    // Trigger
    std::move(p).SetValue(7);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(ThenAfter) {
    test_helpers::CPUTimeBudgetGuard budget(0.1);

    auto [f, p] = MakeContract<Unit>();

    std::atomic<bool> done{false};

    auto finally = std::move(f).Then([](Result<Unit>) {
      return After(1s);
    }).Then([](Result<Unit>) {
      return After(500ms);
    }).Then([](Result<Unit>) {
      return After(250ms);
    }).Then([&done](Result<Unit>) -> Unit {
      done = true;
      std::cout << "Finally!" << std::endl;
      return {};
    });

    // Launch
    std::move(p).SetValue({});

    std::this_thread::sleep_for(1250ms);
    ASSERT_FALSE(done);

    std::move(finally).GetValue();
    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(ThenMultiPools) {
    test_helpers::CPUTimeBudgetGuard budget(0.1);

    auto tp1 = MakeStaticThreadPool(2, "tp1");
    auto tp2 = MakeStaticThreadPool(3, "tp2");

    auto compute1 = [tp1](int value) {
      return AsyncVia([value]() {
        ExpectThread("tp1");
        return value * 2;
      }, tp1);
    };

    auto compute2 = [tp2](int value) {
      return AsyncVia([value]() {
        ExpectThread("tp2");
        return value + 1;
      }, tp2);
    };

    auto [f, p] = MakeContract<int>();

    auto pipeline = std::move(f).Then([compute1](Result<int> r) {
      return compute1(r.Value());
    }).Then([compute2](Result<int> r) {
      return compute2(r.Value());
    });

    // Launch
    std::move(p).SetValue(3);

    ASSERT_EQ(std::move(pipeline).GetValue(), 7);

    tp1->Join();
    tp2->Join();
  }

  SIMPLE_TEST(ViaThen) {
    test_helpers::CPUTimeBudgetGuard budget(0.1);

    auto tp1 = MakeStaticThreadPool(2, "tp1");
    auto tp2 = MakeStaticThreadPool(3, "tp2");

    auto [f, p] = MakeContract<std::string>();

    auto stage = [](int index, std::string thread_label) {
      return [index, thread_label](Result<std::string> r) {
        ExpectThread(thread_label);
        std::cout << "At stage " << index << std::endl;
        return r.Value() + std::to_string(index);
      };
    };

    auto finally = std::move(f)
    .Via(tp1).Then(stage(1, "tp1")).Then(stage(2, "tp1"))
    .Via(tp2).Then(stage(3, "tp2")).Then(stage(4, "tp2"))
    .Via(tp1).Then(stage(5, "tp1"));

    std::move(p).SetValue("0");

    ASSERT_EQ(std::move(finally).GetValue(), "012345");

    tp1->Join();
    tp2->Join();
  }

  SIMPLE_TEST(MessWithExecutors) {
    static const size_t kThreads = 4;

    auto tp = MakeStaticThreadPool(kThreads, "tp");
    auto strand = MakeStrand(tp);

    std::atomic<size_t> total = 0;
    size_t total_strand = 0;

    for (size_t i = 0; i < kThreads; ++i) {
      tp->Execute([]() {
        std::this_thread::sleep_for(100ms);
      });
    }

    static const size_t kPipelines = 123456;

    for (size_t i = 0; i < kPipelines; ++i) {
      Promise<Unit> p;

      p.MakeFuture().Via(tp).Then([&total](Result<Unit>) {
        total.fetch_add(1);
        return Unit{};
      }).Via(strand).Then([&total_strand](Result<Unit>) {
        ++total_strand;
        return Unit{};
      });

      // Launch pipeline
      std::move(p).SetValue({});
    }

    tp->Join();

    ASSERT_EQ(total.load(), kPipelines);
    ASSERT_EQ(total_strand, kPipelines);
  }
}
