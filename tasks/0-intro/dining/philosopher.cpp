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
}

void Philosopher::Eat() {
  table_.AccessPlate(seat_);
  table_.AccessPlate(table_.ToRight(seat_));
  ++eat_count_;
}

// Releases left_fork_ and right_fork_
void Philosopher::ReleaseForks() {
  // Your code goes here
}

void Philosopher::Think() {
  twist::fault::InjectFault();
}

}  // namespace dining
