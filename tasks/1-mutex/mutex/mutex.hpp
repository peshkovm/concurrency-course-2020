#pragma once

#include <twist/strand/spin_wait.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/twisted/futex.hpp>

namespace solutions {

using twist::strand::SpinWait;
using twist::twisted::Futex;

class Mutex {
 public:
  void Lock() {
    int c;
    if ((c = Cmpxchg(0, 1)) != 0) {
      if (c != 2) {
        c = Xchg(2);
      }
      while (c != 0) {
        futex_.Wait(2);
        c = Xchg(2);
      }
    }
  }

  void Unlock() {
    // Your code goes here
    if (Xchg(0) == 2) {
      futex_.WakeOne();
    }
  }

  uint32_t Cmpxchg(uint32_t expected, uint32_t desired) {
    state_.compare_exchange_strong(expected, desired);
    return expected;
  }

  uint32_t Xchg(uint32_t desired) {
    return state_.exchange(desired);
  }

 private:
  twist::stdlike::atomic<uint32_t> state_{0};
  Futex futex_{state_};
};

}  // namespace solutions
