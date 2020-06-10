#pragma once

#include "atomics.hpp"

#include <twist/strand/stdlike.hpp>
#include <twist/strand/spin_wait.hpp>

using twist::strand::SpinWait;

namespace solutions {

// Simple Test-and-Set (TAS) spinlock

class SpinLock {
 public:
  void Lock() {
    SpinWait spin_wait;
    while (AtomicExchange(&locked_, 1)) {
      spin_wait();
    }
  }

  bool TryLock() {
    return !AtomicExchange(&locked_, 1);  // Not implemented
  }

  void Unlock() {
    AtomicStore(&locked_, 0);
  }

 private:
  volatile std::int64_t locked_ = 0;
};

}  // namespace solutions
