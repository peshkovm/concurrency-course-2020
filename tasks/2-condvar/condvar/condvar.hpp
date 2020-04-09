#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/twisted/futex.hpp>

namespace solutions {

class ConditionVariable {
 public:
  ConditionVariable() {
    // Not implemented
  }

  template <class Mutex>
  void Wait(Mutex& mutex) {
    // Not implemented
    // Use mutex.unlock() / mutex.lock() to unlock/lock mutex
  }

  void NotifyOne() {
    // Not implemented
  }

  void NotifyAll() {
    // Not implemented
  }

 private:
  // Your code goes here
};

}  // namespace solutions
