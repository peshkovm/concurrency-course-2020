#include <twist/test_framework/test_framework.hpp>

#include <tinyfutures/executors/thread_label.hpp>
#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>
#include <tinyfutures/executors/work.hpp>

#include "helpers.hpp"

using namespace tiny::executors;
using namespace std::chrono_literals;

#include <thread>

TEST_SUITE_WITH_PRIORITY(Work, 3) {
  SIMPLE_TEST(NestedExecute) {
    auto tp1 = MakeStaticThreadPool(3, "tp1");
    auto tp2 = MakeStaticThreadPool(2, "tp2");

    bool done = false;

    tp1->Execute([&, work = MakeWorkFor(tp2)]() {
      ASSERT_EQ(GetThreadLabel(), "tp1");
      std::this_thread::sleep_for(1s);
      tp2->Execute([&]() {
        ASSERT_EQ(GetThreadLabel(), "tp2");
        done = true;
      });
    });

    tp2->Join();

    ASSERT_TRUE(done);

    tp1->Join();
  }

  SIMPLE_TEST(NestedExecute2) {
    auto tp1 = MakeStaticThreadPool(3, "tp1");
    auto tp2 = MakeStaticThreadPool(2, "tp2");
    auto strand2 = MakeStrand(tp2);

    bool done = false;

    tp1->Execute([&, work = MakeWorkFor(strand2)]() {
      ASSERT_EQ(GetThreadLabel(), "tp1");
      std::this_thread::sleep_for(1s);
      strand2->Execute([&]() {
        ASSERT_EQ(GetThreadLabel(), "tp2");
        done = true;
      });
    });

    tp2->Join();

    ASSERT_TRUE(done);

    tp1->Join();
  }

  SIMPLE_TEST(Counting) {
    auto tp1 = MakeStaticThreadPool(1, "tp1");
    auto tp2 = MakeStaticThreadPool(1, "tp2");
    auto tp3 = MakeStaticThreadPool(1, "tp3");

    tp1->Execute([tp3, work = MakeWorkFor(tp3)]() {
      ExpectThread("tp1");
      std::this_thread::sleep_for(500ms);
      tp3->Execute([]() {
        ExpectThread("tp3");
      });
    });

    tp2->Execute([tp3, work = MakeWorkFor(tp3)]() {
      ExpectThread("tp2");
      std::this_thread::sleep_for(1s);
      tp3->Execute([]() {
        ExpectThread("tp3");
      });
    });

    test_helpers::CPUTimeMeter cpu_time_meter;
    tp3->Join();
    ASSERT_TRUE(cpu_time_meter.UsageSeconds() < 0.1);

    ASSERT_EQ(tp3->ExecutedTaskCount(), 2);

    tp1->Join();
    tp2->Join();
  }
}
