#include "toyalloc.hpp"

#include <twist/support/random.hpp>

#include <twist/fault//adversary/inject_fault.hpp>

#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/executor.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <vector>

#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

////////////////////////////////////////////////////////////////////////////////

void PutCanary(void* addr, size_t thread) {
  *(size_t*)addr = thread;
}

void CheckCanary(void *addr, size_t thread) {
  ASSERT_EQ(*(size_t*)addr, thread);
}

namespace stress {
  class Tester {
   public:
    Tester(const TTestParameters& parameters)
        : parameters_(parameters),
          start_barrier_(parameters.Get(0) + 1) {
    }

    // One-shot
    void Run() {
      twist::test_utils::ScopedExecutor executor;
      for (size_t t = 0; t < parameters_.Get(0); ++t) {
        executor.Submit(&Tester::ThreadAllocate, this, t);
      }
      executor.Submit(&Tester::ThreadFork, this);
    }

   private:
    void ThreadAllocate(size_t thread_index) {
      start_barrier_.PassThrough();

      size_t batch_limit = parameters_.Get(1);

      while (!done_.load()) {
        // Allocate
        size_t batch_size = twist::RandomUInteger(batch_limit);

        std::vector<void*> allocated;

        for (size_t j = 0; j < batch_size; ++j) {
          void* addr = toyalloc::Allocate();
          PutCanary(addr, thread_index);
          allocated.push_back(addr);
        }

        // Release
        for (void* addr : allocated) {
          CheckCanary(addr, thread_index);
          toyalloc::Free(addr);
        }
      }
    }

    void ThreadFork() {
      start_barrier_.PassThrough();

      size_t forks = parameters_.Get(2);
      for (size_t i = 0; i < forks; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
          // Child

          TestTimeLimitWatcher countdown(std::chrono::seconds(3));
          (void)toyalloc::Allocate();
          std::_Exit(0);  // Quick exit
        } else if (pid > 0) {
          // Parent

          // Await child process
          int status;
          (void)waitpid(pid, &status, 0);
          if (!WIFEXITED(status)) {
            FailTest("Forked process failed to allocate block");
          }
        }
      }

      done_.store(true);
    }

   private:
    TTestParameters parameters_;
    twist::test_utils::OnePassBarrier start_barrier_;
    std::atomic<bool> done_{false};
  };

};

void ForkStressTest(TTestParameters parameters) {
  stress::Tester(parameters).Run();
}

// Parameters: alloc threads, alloc batch limit, forks

T_TEST_CASES(ForkStressTest)
  .TimeLimit(std::chrono::seconds(30))
  .Case({1, 100, 100})
  .Case({5, 50, 100})
  .Case({10, 100, 100});

////////////////////////////////////////////////////////////////////////////////

void InitAllocator() {
  static const size_t kArenaPages = 1024;
  auto arena = twist::MmapAllocation::AllocatePages(kArenaPages);
  toyalloc::Init(std::move(arena));
}

int main() {
  InitAllocator();
  RunTests(ListAllTests());
}
