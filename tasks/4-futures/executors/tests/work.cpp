#include <twist/test_framework/test_framework.hpp>

#include <tinyfutures/executors/thread_label.hpp>
#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>
#include <tinyfutures/executors/work.hpp>

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
}
