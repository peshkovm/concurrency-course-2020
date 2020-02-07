#pragma once

#include "dining.hpp"

namespace dining {

class Philosopher {
 public:
  Philosopher(Table& table, size_t seat)
      : table_(table),
        seat_(seat),
        left_fork_(table_.LeftFork(seat)),
        right_fork_(table_.RightFork(seat)) {
  }

  void EatOneMoreTime();

  size_t EatCount() const {
    return eat_count_;
  }

 private:
  void AcquireForks();
  void Eat();
  void ReleaseForks();
  void Think();

 private:
  Table& table_;
  size_t seat_;

  Fork& left_fork_;
  Fork& right_fork_;

  size_t eat_count_ = 0;
};

}  // namespace dining
