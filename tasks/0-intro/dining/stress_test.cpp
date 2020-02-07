#include "dining.hpp"
#include "philosopher.hpp"

#include <twist/strand/test.hpp>

#include <twist/test_utils/barrier.hpp>
#include <twist/test_utils/executor.hpp>

#include <vector>

namespace dining {

class DiningTester {
 public:
  DiningTester(const TTestParameters& parameters)
    : parameters_(parameters),
      start_barrier_(parameters_.Get(0)),
      table_(parameters_.Get(0)) {
  }

  void Run() {
    Dining();
    CheckStarvation();
  }

 private:
  void Dining() {
    size_t seats = parameters_.Get(0);
    for (size_t seat = 0; seat < seats; ++seat) {
      philosophers_.emplace_back(table_, seat);
    }

    twist::test_utils::ScopedExecutor executor;
    for (size_t seat = 0; seat < seats; ++seat) {
      executor.Submit(&DiningTester::RunPhilosopherThread, this, seat);
    }
  }

  void RunPhilosopherThread(size_t seat) {
    auto& philosopher = philosophers_[seat];

    start_barrier_.PassThrough();

    size_t iterations = parameters_.Get(1);
    for (size_t i = 0; i < iterations; ++i) {
      philosopher.EatOneMoreTime();
    }
  }

  void CheckStarvation() {
    size_t iterations = parameters_.Get(1);
    for (const auto& philosopher : philosophers_) {
      ASSERT_TRUE_M(philosopher.EatCount() > iterations / 5, "Starvation");
    }
  }

 private:
  TTestParameters parameters_;
  twist::test_utils::OnePassBarrier start_barrier_;
  Table table_;
  std::vector<Philosopher> philosophers_;
};

}  // namespace dining

void PhilosophersStressTest(TTestParameters parameters) {
  dining::DiningTester(parameters).Run();
}


// Parameters: Threads (philosophers), iterations

T_TEST_CASES(PhilosophersStressTest)
  .TimeLimit(std::chrono::seconds(10))
  .Case({2, 10000})
  .TimeLimit(std::chrono::seconds(30))
  .Case({3, 50000})
  .Case({5, 25000})
  .Case({10, 10000});

RUN_ALL_TESTS();
