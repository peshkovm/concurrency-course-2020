#include "philosopher.hpp"

#include <twist/fault/adversary/inject_fault.hpp>

namespace dining {

void Philosopher::EatOneMoreTime() {
  AcquireForks();
  Eat();
  ReleaseForks();
  Think();
}

// Acquires left_fork_ and right_fork_
void Philosopher::AcquireForks() {
  // Your code goes here
  if (seat_ == 0) {
    right_fork_.lock();
    left_fork_.lock();
  } else {
    left_fork_.lock();
    right_fork_.lock();
  }
}

void Philosopher::Eat() {
  table_.AccessPlate(seat_);
  table_.AccessPlate(table_.ToRight(seat_));
  ++eat_count_;
}

// Releases left_fork_ and right_fork_
void Philosopher::ReleaseForks() {
  // Your code goes here
  left_fork_.unlock();
  right_fork_.unlock();
}

void Philosopher::Think() {
  twist::fault::InjectFault();
}

}  // namespace dining
