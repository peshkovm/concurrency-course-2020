#include <twist/test_framework/test_framework.hpp>

#include <tinyfutures/executors/static_thread_pool.hpp>
#include <tinyfutures/executors/strand.hpp>
#include <tinyfutures/executors/thread_label.hpp>

#include "helpers.hpp"

#include <thread>
#include <atomic>

using namespace tiny::executors;
using namespace std::chrono_literals;

TEST_SUITE_WITH_PRIORITY(Strand, 2) {

  SIMPLE_TEST(ExecuteTask) {
    auto tp = MakeStaticThreadPool(4, "strands");

    auto strand = MakeStrand(tp);

    bool done{false};

    strand->Execute([&done]() {
      ExpectThread("strands");
      done = true;
    });

    tp->Join();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Counter) {
    auto tp = MakeStaticThreadPool(13, "pool");

    size_t counter = 0;

    auto strand = MakeStrand(tp);

    static const size_t kIncrements = 65536;

    for (size_t i = 0; i < kIncrements; ++i) {
      strand->Execute([&counter]() {
        ExpectThread("pool");
        ++counter;
      });
    };

    tp->Join();

    ASSERT_EQ(counter, kIncrements);
  }

  SIMPLE_TEST(Fifo) {
    auto tp = MakeStaticThreadPool(13, "pool");

    size_t next_ticket = 0;

    auto strand = MakeStrand(tp);

    static const size_t kTickets = 123456;

    for (size_t t = 0; t < kTickets; ++t) {
      strand->Execute([&next_ticket, t]() {
        ExpectThread("pool");
        ASSERT_EQ(next_ticket, t);
        ++next_ticket;
      });
    };

    tp->Join();

    ASSERT_EQ(next_ticket, kTickets);
  }

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

  SIMPLE_TEST(ConcurrentStrands) {
    auto tp = MakeStaticThreadPool(16, "pool");

    static const size_t kStrands = 50;

    std::vector<Counter> counters;
    counters.reserve(kStrands);
    for (size_t i = 0; i < kStrands; ++i) {
      counters.emplace_back(tp);
    }

    static const size_t kBatchSize = 25;
    static const size_t kIterations = 25;
    for (size_t i = 0; i < kIterations; ++i) {
      for (size_t j = 0; j < kStrands; ++j) {
        for (size_t k = 0; k < kBatchSize; ++k) {
          counters[j].Increment();
        }
      }
    }

    tp->Join();

    for (size_t i = 0; i < kStrands; ++i) {
      ASSERT_EQ(counters[i].Value(), kBatchSize * kIterations);
    }
  }

  SIMPLE_TEST(ConcurrentExecutes) {
    auto tp = MakeStaticThreadPool(2, "test");
    auto strand = MakeStrand(tp);

    static const size_t kProducers = 5;
    static const size_t kTasks = 1024;

    test_helpers::OnePassBarrier barrier{kProducers};
    std::atomic<int> done{0};

    auto task = [&done]() {
      ExpectThread("test");
      done.fetch_add(1);
    };

    std::vector<std::thread> producers;

    for (size_t i = 0; i < kProducers; ++i) {
      producers.emplace_back([strand, &task, &barrier]() {
        barrier.Arrive();
        for (size_t j = 0; j < kTasks; ++j) {
          strand->Execute(task);
        }
      });
    }

    for (auto& t : producers) {
      t.join();
    }

    tp->Join();
    ASSERT_EQ(done.load(), kProducers * kTasks);
  }


  SIMPLE_TEST(Batching) {
    auto tp = MakeStaticThreadPool(1, "tp");

    tp->Execute([]() {
      // bubble
      std::this_thread::sleep_for(1s);
    });

    auto strand = MakeStrand(tp);

    static const size_t kStrandTasks = 100;

    size_t completed = 0;
    for (size_t i = 0; i < kStrandTasks; ++i) {
      strand->Execute([&completed]() {
        ++completed;
      });
    };

    tp->Join();

    ASSERT_EQ(completed, kStrandTasks);
    ASSERT_TRUE(tp->ExecutedTaskCount() < 5);
  }

  SIMPLE_TEST(StrandOverStrand) {
    auto tp = MakeStaticThreadPool(4, "strands");

    auto strand = MakeStrand(MakeStrand(MakeStrand(tp)));

    bool done = false;
    strand->Execute([&done]() {
      done = true;
    });

    tp->Join();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(KeepStrongRef) {
    auto tp = MakeStaticThreadPool(1, "test");

    tp->Execute([]() {
      // bubble
      std::this_thread::sleep_for(1s);
    });

    bool done = false;
    MakeStrand(tp)->Execute([&done]() {
      done = true;
    });

    tp->Join();
    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(DoNotOccupyThread) {
    auto tp = MakeStaticThreadPool(1, "thread");

    auto strand = MakeStrand(tp);

    tp->Execute([]() {
      // bubble
      std::this_thread::sleep_for(1s);
    });

    std::atomic<bool> stop{false};

    static const auto kStepPause = 10ms;

    auto step = []() {
      ExpectThread("thread");
      std::this_thread::sleep_for(kStepPause);
    };

    for (size_t i = 0; i < 100; ++i) {
      strand->Execute(step);
    }

    tp->Execute([&stop]() {
      stop.store(true);
    });

    while (!stop.load()) {
      strand->Execute(step);
      std::this_thread::sleep_for(kStepPause);
    }

    tp->Join();
  }

  SIMPLE_TEST(Exceptions) {
    auto tp = MakeStaticThreadPool(1, "pool");
    auto strand = MakeStrand(tp);

    tp->Execute([]() {
      std::this_thread::sleep_for(1s);
    });

    bool done = false;

    strand->Execute([]() {
      throw std::runtime_error("You shall not pass!");
    });
    strand->Execute([&done]() {
      done = true;
    });

    tp->Join();
    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(NonBlockingExecute) {
    auto tp = MakeStaticThreadPool(1, "tp");
    auto strand = MakeStrand(tp);

    strand->Execute([]() {
      ExpectThread("tp");
      std::this_thread::sleep_for(2s);
    });

    std::this_thread::sleep_for(500ms);

    test_helpers::StopWatch stop_watch;
    strand->Execute([]() {
      ExpectThread("tp");
    });
    ASSERT_LE(stop_watch.Elapsed(), 100ms);

    tp->Join();
  }
}
