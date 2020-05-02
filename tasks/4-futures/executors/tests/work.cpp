#include <twist/test_framework/test_framework.hpp>

#include <tinyfutures/executors/thread_label.hpp>
#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>
#include <tinyfutures/executors/work.hpp>

#include "helpers.hpp"

using namespace tiny::executors;
using namespace std::chrono_literals;

#include <thread>
#include <atomic>

TEST_SUITE_WITH_PRIORITY(Work, 3) {
  SIMPLE_TEST(CrossPoolExecute) {
    auto tp1 = MakeStaticThreadPool(3, "tp1");
    auto tp2 = MakeStaticThreadPool(2, "tp2");

    bool done = false;

    auto tp2_task = [&done]() {
      ExpectThread("tp2");
      done = true;
    };

    tp1->Execute([tp2 = KeepWorking(tp2), tp2_task]() {
      ExpectThread("tp1");
      std::this_thread::sleep_for(1s);
      tp2->Execute(tp2_task);
    });

    tp2->Join();

    ASSERT_TRUE(done);

    tp1->Join();
  }

  SIMPLE_TEST(CrossPoolExecute2) {
    auto tp1 = MakeStaticThreadPool(3, "tp1");
    auto tp2 = MakeStaticThreadPool(2, "tp2");
    auto strand2 = MakeStrand(tp2);

    bool done = false;

    auto tp2_task = [&done]() {
      ExpectThread("tp2");
      done = true;
    };

    tp1->Execute([tp2_task, strand2 = KeepWorking(strand2)]() {
      ExpectThread("tp1");
      std::this_thread::sleep_for(1s);
      strand2->Execute(tp2_task);
    });

    tp2->Join();

    ASSERT_TRUE(done);

    tp1->Join();
  }

  SIMPLE_TEST(MorePools) {
    auto tp1 = MakeStaticThreadPool(1, "tp1");
    auto tp2 = MakeStaticThreadPool(1, "tp2");
    auto tp3 = MakeStaticThreadPool(1, "tp3");

    auto tp3_task = []() {
      ExpectThread("tp3");
    };

    tp1->Execute([tp3_task, tp3 = KeepWorking(tp3)]() {
      ExpectThread("tp1");
      std::this_thread::sleep_for(500ms);
      tp3->Execute(tp3_task);
    });

    tp2->Execute([tp3_task, tp3 = KeepWorking(tp3)]() {
      ExpectThread("tp2");
      std::this_thread::sleep_for(1s);
      tp3->Execute(tp3_task);
    });

    {
      test_helpers::CPUTimeBudgetGuard budget(0.1);
      tp3->Join();
    }

    ASSERT_EQ(tp3->ExecutedTaskCount(), 2);

    tp1->Join();
    tp2->Join();
  }

  SIMPLE_TEST(ConcurrentWork) {
    auto tp = MakeStaticThreadPool(3, "test");

    std::atomic<size_t> completed{0};

    auto add_work = [&]() {
      tp->WorkCreated();
      std::this_thread::sleep_for(1s);
      tp->WorkCompleted();
      ++completed;
    };

    std::thread t1(add_work);
    std::thread t2(add_work);

    std::this_thread::sleep_for(500ms);

    tp->Join();
    ASSERT_EQ(completed.load(), 2);

    t1.join();
    t2.join();
  }
}
