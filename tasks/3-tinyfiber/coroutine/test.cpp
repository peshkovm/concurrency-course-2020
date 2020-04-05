#include "scheduler.hpp"
#include "coroutine.hpp"
#include "fiber.hpp"

#include <twist/test_framework/test_framework.hpp>

#include <atomic>
#include <chrono>
#include <memory>

using namespace std::chrono_literals;

using tinyfiber::ThreadPool;
using tinyfiber::Spawn;
using tinyfiber::Yield;
using tinyfiber::FiberRoutine;

TEST_SUITE(ThreadPool) {
  SIMPLE_TEST(HelloWorld) {
    ThreadPool tp{1};
    tp.Submit([]() {
      std::this_thread::sleep_for(1s);
      std::cout << "Hello, World!" << std::endl;
    });
    tp.Join();
  }

  SIMPLE_TEST(WeNeedToGoDeeper) {
    ThreadPool tp{1};
    auto deeper = []() {
      ThreadPool::Current()->Submit(
          []() {
            std::cout << "We need to go deeper..." << std::endl;
          });
    };
    tp.Submit(deeper);
    tp.Join();
  }

  SIMPLE_TEST(SubmitAferJoin) {
    ThreadPool tp{3};

    auto submit = [&tp]() {
      std::this_thread::sleep_for(3s);

      // After join
      tp.Submit([]() {
        std::this_thread::sleep_for(1s);
        std::cout << "Submitted after Join" << std::endl;
      });
    };

    tp.Submit(submit);

    tp.Join();
  }

  SIMPLE_TEST(Current) {
    ThreadPool tp{4};
    ASSERT_EQ(ThreadPool::Current(), nullptr);

    auto task = [&tp]() {
      ASSERT_EQ(ThreadPool::Current(), &tp);
    };

    tp.Submit(task);
    tp.Join();
  }
}

struct TreeNode;

using TreeNodePtr = std::shared_ptr<TreeNode>;

struct TreeNode {
  TreeNodePtr left_;
  TreeNodePtr right_;

  TreeNode(TreeNodePtr left = nullptr, TreeNodePtr right = nullptr)
    : left_(std::move(left)), right_(std::move(right)) {
  }

  static TreeNodePtr Create(TreeNodePtr left, TreeNodePtr right) {
    return std::make_shared<TreeNode>(std::move(left), std::move(right));
  }

  static TreeNodePtr CreateLeaf() {
    return std::make_shared<TreeNode>();
  }
};

namespace coroutine = tinyfiber::coroutine;

TEST_SUITE(Coroutine) {
  SIMPLE_TEST(JustWorks) {
    auto foo_routine = [&]() {
      coroutine::Suspend();
    };

    coroutine::Coroutine foo(foo_routine);

    ASSERT_FALSE(foo.IsCompleted());
    foo.Resume();
    foo.Resume();
    ASSERT_TRUE(foo.IsCompleted());

    ASSERT_THROW(foo.Resume(), coroutine::CoroutineCompleted)
  }

  SIMPLE_TEST(Interleaving) {
    int step = 0;

    auto finn_routine = [&]() {
      ASSERT_EQ(step, 0);
      step = 1;
      coroutine::Suspend();
      ASSERT_EQ(step, 2);
      step = 3;
    };

    auto jake_routine = [&]() {
      ASSERT_EQ(step, 1);
      step = 2;
      coroutine::Suspend();
      ASSERT_EQ(step, 3);
      step = 4;
    };

    coroutine::Coroutine finn(finn_routine);
    coroutine::Coroutine jake(jake_routine);

    finn.Resume();
    jake.Resume();

    ASSERT_EQ(step, 2);

    finn.Resume();
    jake.Resume();

    ASSERT_TRUE(finn.IsCompleted());
    ASSERT_TRUE(jake.IsCompleted());

    ASSERT_EQ(step, 4);
  }

  void TreeWalk(TreeNodePtr node) {
    if (node->left_) {
      TreeWalk(node->left_);
    }
    coroutine::Suspend();
    if (node->right_) {
      TreeWalk(node->right_);
    }
  }

  SIMPLE_TEST(TreeWalk) {
    TreeNodePtr root = TreeNode::Create(
      TreeNode::CreateLeaf(),
      TreeNode::Create(
        TreeNode::Create(
          TreeNode::CreateLeaf(),
          TreeNode::CreateLeaf()
        ),
        TreeNode::CreateLeaf()
      )
    );

    coroutine::Coroutine walker([&root]() {
      TreeWalk(root);
    });

    size_t node_count = 0;

    while (true) {
      walker.Resume();
      if (walker.IsCompleted()) {
        break;
      }
      ++node_count;
    }

    ASSERT_EQ(node_count, 7);
  }

  SIMPLE_TEST(Pipeline) {
    const size_t kSteps = 123;

    size_t inner_step_count = 0;

    auto inner_routine = [&]() {
      for (size_t i = 0; i < kSteps; ++i) {
        ++inner_step_count;
        coroutine::Suspend();
      }
    };

    auto outer_routine = [&]() {
      coroutine::Coroutine inner(inner_routine);
      while (!inner.IsCompleted()) {
        inner.Resume();
        coroutine::Suspend();
      }
    };

    coroutine::Coroutine outer(outer_routine);
    while (!outer.IsCompleted()) {
      outer.Resume();
    }

    ASSERT_EQ(inner_step_count, kSteps);
  }

  SIMPLE_TEST(NotInCoroutine) {
    ASSERT_THROW(coroutine::Suspend(), coroutine::NotInCoroutine)
  }

  SIMPLE_TEST(Exception) {
    auto foo_routine = [&]() {
      coroutine::Suspend();
      throw std::runtime_error("Test exception");
    };

    coroutine::Coroutine foo(foo_routine);

    ASSERT_FALSE(foo.IsCompleted());
    foo.Resume();
    ASSERT_THROW(foo.Resume(), std::runtime_error);
    ASSERT_TRUE(foo.IsCompleted());
  }

  struct MyException {
  };

  SIMPLE_TEST(NestedException1) {
    auto bar_routine = [&]() {
      throw MyException();
    };

    auto foo_routine = [&]() {
      coroutine::Coroutine bar(bar_routine);
      ASSERT_THROW(bar.Resume(), MyException);
    };

    coroutine::Coroutine foo(foo_routine);
    foo.Resume();
  }

  SIMPLE_TEST(NestedException2) {
    auto bar_routine = [&]() {
      throw MyException();
    };

    auto foo_routine = [&]() {
      coroutine::Coroutine bar(bar_routine);
      bar.Resume();
    };

    coroutine::Coroutine foo(foo_routine);
    ASSERT_THROW(foo.Resume(), MyException);
  }

  SIMPLE_TEST(Leak) {
    auto shared_ptr = std::make_shared<int>(42);
    std::weak_ptr<int> weak_ptr = shared_ptr;

    {
      auto routine = [ptr = std::move(shared_ptr)]() {};
      coroutine::Coroutine co(routine);
      co.Resume();
    }

    ASSERT_FALSE(weak_ptr.lock());
  }
}

static void RunScheduler(tinyfiber::FiberRoutine init, size_t threads) {
  tinyfiber::ThreadPool thread_pool{threads};
  tinyfiber::Spawn(init, thread_pool);
  thread_pool.Join();
}

TEST_SUITE(Fiber) {
  SIMPLE_TEST(InsideThreadPool) {
    ThreadPool tp{3};
    std::atomic<bool> done{false};

    auto tester = [&]() {
      ASSERT_EQ(ThreadPool::Current(), &tp);
      done.store(true);
    };

    Spawn(tester, tp);
    tp.Join();

    ASSERT_EQ(done.load(), true);
  }

  SIMPLE_TEST(ChildInsideThreadPool) {
    ThreadPool tp{3};
    std::atomic<size_t> done{0};

    auto tester = [&tp, &done]() {
      ASSERT_EQ(ThreadPool::Current(), &tp);

      auto child = [&tp, &done]() {
        ASSERT_EQ(ThreadPool::Current(), &tp);
        done.fetch_add(1);
      };
      Spawn(child);

      done.fetch_add(1);
    };

    Spawn(tester, tp);
    tp.Join();

    ASSERT_EQ(done.load(), 2);
  }

  SIMPLE_TEST(RunInParallel) {
    ThreadPool tp{3};
    std::atomic<size_t> completed{0};

    auto sleeper = [&completed]() {
      std::this_thread::sleep_for(3s);
      completed.fetch_add(1);
    };

    twist::Timer timer;

    Spawn(sleeper, tp);
    Spawn(sleeper, tp);
    Spawn(sleeper, tp);
    tp.Join();

    ASSERT_EQ(completed.load(), 3);
    ASSERT_TRUE(timer.Elapsed() < 3s + 500ms);
  }

  SIMPLE_TEST(Yield) {
    std::atomic<int> value{0};

    auto check_value = [&value]() {
      const int kLimit = 10;

      ASSERT_TRUE(value.load() < kLimit);
      ASSERT_TRUE(value.load() > -kLimit);
    };

    static const size_t kIterations = 12345;

    auto bull = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        value.fetch_add(1);
        Yield();
        check_value();
      }
    };

    auto bear = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        value.fetch_sub(1);
        Yield();
        check_value();
      }
    };

    auto starter = [&]() {
      Spawn(bull);
      Spawn(bear);
    };

    ThreadPool tp{1};
    Spawn(starter, tp);
    tp.Join();
  }

  SIMPLE_TEST(Yield2) {
    ThreadPool tp{4};

    static const size_t kYields = 123456;

    auto tester = []() {
      for (size_t i = 0; i < kYields; ++i) {
        Yield();
      }
    };

    Spawn(tester, tp);
    Spawn(tester, tp);

    tp.Join();
  }

  class ForkTester {
   public:
    ForkTester(size_t threads)
      : pool_(threads) {
    }

    size_t Explode(size_t d) {
      Spawn(MakeForker(d), pool_);
      pool_.Join();
      return leafs_.load();
    }

   private:
    FiberRoutine MakeForker(size_t d) {
      return [this, d]() {
        if (d > 2) {
          Spawn(MakeForker(d - 2));
          Spawn(MakeForker(d - 1));
        } else {
          leafs_.fetch_add(1);
        }
      };
    }

   private:
    ThreadPool pool_;
    std::atomic<size_t> leafs_{0};
  };

  SIMPLE_TEST(Forks) {
    ForkTester tester{4};
    ASSERT_EQ(tester.Explode(21), 10946);
  }

  SIMPLE_TEST(TwoPools1) {
    ThreadPool tp_1{4};
    ThreadPool tp_2{4};

    auto make_tester = [](ThreadPool& tp) {
      return [&tp]() {
        ASSERT_EQ(ThreadPool::Current(), &tp);
      };
    };

    Spawn(make_tester(tp_1), tp_1);
    Spawn(make_tester(tp_2), tp_2);
  }

  SIMPLE_TEST(TwoPools2) {
    ThreadPool tp_1{4};
    ThreadPool tp_2{4};

    auto make_tester = [](ThreadPool& tp) {
      return [&tp]() {
        static const size_t kIterations = 1024;

        for (size_t i = 0; i < kIterations; ++i) {
          ASSERT_EQ(ThreadPool::Current(), &tp);

          Yield();

          Spawn([&tp]() {
            ASSERT_EQ(ThreadPool::Current(), &tp);
          });
        }
      };
    };

    Spawn(make_tester(tp_1), tp_1);
    Spawn(make_tester(tp_2), tp_2);
  }

  struct RacyCounter {
   public:
    void Increment() {
      value_.store(
          value_.load(std::memory_order_relaxed) + 1,
          std::memory_order_relaxed);
    }
    size_t Get() const {
      return value_.load(std::memory_order_relaxed);
    }
   private:
    std::atomic<size_t> value_{0};
  };

  SIMPLE_TEST(RacyCounter) {
    static const size_t kIncrements = 100'000;
    static const size_t kThreads = 4;
    static const size_t kFibers = 100;

    RacyCounter counter;

    auto routine = [&]() {
      for (size_t i = 0; i < kIncrements; ++i) {
        counter.Increment();
        if (i % 10 == 0) {
          tinyfiber::Yield();
        }
      }
    };

    auto init = [&]() {
      for (size_t i = 0; i < kFibers; ++i) {
        tinyfiber::Spawn(routine);
      }
    };

    RunScheduler(init, kThreads);

    std::cout << "Thread count: " << kThreads << std::endl
              << "Fibers: " << kFibers << std::endl
              << "Increments per fiber: " << kIncrements << std::endl
              << "Racy counter value: " << counter.Get() << std::endl;

    ASSERT_GE(counter.Get(), kIncrements);
    ASSERT_LT(counter.Get(), kIncrements * kFibers);
  }
}

RUN_ALL_TESTS()
