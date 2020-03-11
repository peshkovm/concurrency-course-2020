#include "scheduler.hpp"
#include "coroutine.hpp"
#include "fiber.hpp"

#include <twist/test_framework/test_framework.hpp>

#include <memory>

using namespace std::chrono_literals;

TEST_SUITE(Scheduler) {
  SIMPLE_TEST(HelloWorld) {
    tinyfiber::Scheduler scheduler{1};
    scheduler.Submit([]() {
      std::this_thread::sleep_for(1s);
      std::cout << "Hello, World!" << std::endl;
    });
    scheduler.Shutdown();
  }

  SIMPLE_TEST(WeNeedToGoDeeper) {
    tinyfiber::Scheduler scheduler{1};
    auto deeper = []() {
      tinyfiber::Scheduler::Current()->Submit(
          []() {
            std::cout << "We need to go deeper..." << std::endl;
          });
    };
    scheduler.Submit(deeper);
    scheduler.Shutdown();
  }

  SIMPLE_TEST(SubmitAfterShutdown) {
    tinyfiber::Scheduler scheduler{3};
    auto sleep = []() {
      std::this_thread::sleep_for(3s);
    };
    scheduler.Submit(sleep);
    scheduler.Shutdown();
    std::this_thread::sleep_for(1s);
    scheduler.Submit([]() {
      std::this_thread::sleep_for(1s);
      std::cout << "After shutdown" << std::endl;
    });
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
}

TEST_SUITE(Fiber) {
  SIMPLE_TEST(PingPong) {
    int step = 0;

    auto finn = [&]() {
      ASSERT_EQ(step, 0);
      step = 1;
      tinyfiber::Yield();
      ASSERT_EQ(step, 2);
      step = 3;
    };

    auto jake = [&]() {
      ASSERT_EQ(step, 1);
      step = 2;
      tinyfiber::Yield();
      ASSERT_EQ(step, 3);
      step = 4;
    };

    tinyfiber::RunScheduler([&]() {
      tinyfiber::Spawn(finn);
      tinyfiber::Spawn(jake);
    });

    ASSERT_EQ(step, 4);
  }

  class Baton {
   public:
    Baton(size_t count)
      : count_(count), current_(0) {
    }

    size_t CurrentOwner() {
      return current_;
    }

    void Transfer() {
      current_ = (current_ + 1) % count_;
    }

   private:
    const size_t count_;
    size_t current_;
  };

  SIMPLE_TEST(RelayRace) {
    const size_t kRunners = 10;
    const size_t kLoops = 10;

    Baton baton(kRunners);

    auto runner = [&](size_t t) {
      for (size_t i = 0; i < kLoops; ++i) {
        ASSERT_EQ(baton.CurrentOwner(), t);
        baton.Transfer();
        tinyfiber::Yield();
      }
    };

    auto ref = [&]() {
      for (size_t i = 0; i < kRunners; ++i) {
        tinyfiber::Spawn(std::bind(runner, i));
      }
    };

    tinyfiber::RunScheduler(ref);

    ASSERT_EQ(baton.CurrentOwner(), 0);
  }

  SIMPLE_TEST(RacyCounter) {
    static const size_t kIncrements = 100'000;
    static const size_t kThreads = 4;
    static const size_t kFibers = 100;

    size_t count = 0;

    auto routine = [&]() {
      for (size_t i = 0; i < kIncrements; ++i) {
        ++count;
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

    tinyfiber::RunScheduler(init, kThreads);

    std::cout << "Thread count: " << kThreads << std::endl
              << "Fibers: " << kFibers << std::endl
              << "Increments per fiber: " << kIncrements << std::endl
              << "Racy counter value: " << count << std::endl;

    ASSERT_GE(count, kIncrements);
    ASSERT_LT(count, kIncrements * kFibers);
  }
}

RUN_ALL_TESTS()
