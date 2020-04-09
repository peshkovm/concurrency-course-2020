#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <cstddef>

namespace solutions {

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t /*participants*/) {
  }

  void Arrive() {
    // Not implemented
  }

 private:
  // Your code goes here
};

}  // namespace solutions
