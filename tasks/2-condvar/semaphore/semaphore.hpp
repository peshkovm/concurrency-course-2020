#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace solutions {

class Semaphore {
 public:
  explicit Semaphore(size_t /*initial_count*/) {
    // Not implemented
  }

  void Acquire() {
    // Not implemented
  }

  void Release() {
    // Not implemented
  }

 private:
  // Your code goes here
};

}  // namespace solutions
