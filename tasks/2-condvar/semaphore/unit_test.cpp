#include "semaphore.hpp"
#include "channel.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/strand/stdlike.hpp>

#include <twist/test_utils/executor.hpp>

#include <twist/support/random.hpp>

#include <atomic>
#include <deque>
#include <chrono>
#include <string>

TEST_SUITE(Semaphore) {
  SIMPLE_T_TEST(NonBlocking) {
    solutions::Semaphore semaphore(2);

    semaphore.Acquire();  // -1
    semaphore.Release();  // +1

    semaphore.Acquire();  // -1
    semaphore.Acquire();  // -1
    semaphore.Release();  // +1
    semaphore.Release();  // +1
  }

  SIMPLE_T_TEST(Buffer) {
    solutions::BufferedChannel<std::string> channel{2};

    channel.Send("hello");
    channel.Send("world");

    ASSERT_EQ(channel.Receive(), "hello");
    ASSERT_EQ(channel.Receive(), "world");
  }

  SIMPLE_T_TEST(Blocking) {
    solutions::Semaphore semaphore(0);

    bool touched = false;

    auto touch_routine = [&touched, &semaphore]() {
      semaphore.Acquire();
      touched = true;
    };

    twist::strand::thread toucher(touch_routine);

    twist::strand::this_thread::sleep_for(
      std::chrono::milliseconds(250));

    ASSERT_FALSE(touched);

    semaphore.Release();
    toucher.join();

    ASSERT_TRUE(touched);
  }

  SIMPLE_T_TEST(PingPong) {
    solutions::Semaphore my{1};
    solutions::Semaphore that{0};

    int step = 0;

    auto opponent_routine = [&]() {
      that.Acquire();
      ASSERT_EQ(step, 1);
      step = 0;
      my.Release();
    };

    twist::strand::thread opponent(opponent_routine);

    my.Acquire();
    ASSERT_EQ(step, 0);
    step = 1;
    that.Release();

    my.Acquire();
    ASSERT_TRUE(step == 0);

    opponent.join();
  }
}

TEST_SUITE(BufferedChannel) {
  SIMPLE_T_TEST(SendThenReceive) {
    solutions::BufferedChannel<int> chan{1};
    chan.Send(42);
    ASSERT_EQ(chan.Receive(), 42);
  }

  SIMPLE_T_TEST(FifoSmall) {
    solutions::BufferedChannel<std::string> chan{2};

    twist::strand::thread producer(
        [&chan]() {
          chan.Send("hello");
          chan.Send("world");
          chan.Send("!");
        });

    ASSERT_EQ(chan.Receive(), "hello");
    ASSERT_EQ(chan.Receive(), "world");
    ASSERT_EQ(chan.Receive(), "!");

    producer.join();
  }

  SIMPLE_T_TEST(Fifo) {
    solutions::BufferedChannel<int> chan{3};

    static const int kItems = 1024;

    auto producer_routine = [&]() {
      for (int i = 0; i < kItems; ++i) {
        chan.Send(i);
      }
      chan.Send(-1);  // Poison pill
    };

    twist::strand::thread producer(producer_routine);

    // Consumer

    for (int i = 0; i < kItems; ++i) {
      ASSERT_EQ(chan.Receive(), i);
    }
    ASSERT_EQ(chan.Receive(), -1);

    producer.join();
  }

  SIMPLE_T_TEST(Capacity) {
    solutions::BufferedChannel<int> chan{3};
    std::atomic<size_t> send_count{0};

    auto producer_routine = [&]() {
      for (size_t i = 0; i < 100; ++i) {
        chan.Send(i);
        send_count.store(i);
      }
      chan.Send(-1);
    };

    twist::strand::thread producer(producer_routine);

    twist::strand::this_thread::sleep_for(
      std::chrono::milliseconds(100));

    ASSERT_TRUE(send_count.load() <= 3);

    for (size_t i = 0; i < 14; ++i) {
      (void)chan.Receive();
    }

    twist::strand::this_thread::sleep_for(
      std::chrono::milliseconds(100));

    ASSERT_TRUE(send_count.load() <= 17);

    while (chan.Receive() != -1) {
      // Pass
    }

    producer.join();
  }

  SIMPLE_T_TEST(Pill) {
    static const size_t kThreads = 10;
    solutions::BufferedChannel<int> chan{1};

    auto routine = [&]() {
      twist::strand::this_thread::sleep_for(
          std::chrono::milliseconds(
              twist::RandomUInteger(1, 1000)));

      ASSERT_EQ(chan.Receive(), -1);
      chan.Send(-1);
    };

    twist::test_utils::ScopedExecutor executor;
    for (size_t i = 0; i < kThreads; ++i) {
      executor.Submit(routine);
    }

    chan.Send(-1);
  }
}

RUN_ALL_TESTS()
