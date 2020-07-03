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
    uint32_t expected = state_.load();
    mutex.unlock();
    futex_.Wait(expected);
    mutex.lock();
  }

  void NotifyOne() {
    state_.fetch_add(1);
    futex_.WakeOne();
    // Not implemented
  }

  void NotifyAll() {
    state_.fetch_add(1);
    futex_.WakeAll();
    // Not implemented
  }

 private:
  // Your code goes here
  twist::stdlike::atomic<uint32_t> state_{0};
  twist::twisted::Futex futex_{state_};
};

}  // namespace solutions
