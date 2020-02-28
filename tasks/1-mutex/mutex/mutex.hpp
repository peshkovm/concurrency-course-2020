#pragma once

#include <twist/strand/spin_wait.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/twisted/futex.hpp>

namespace solutions {

using twist::twisted::Futex;
using twist::strand::SpinWait;

class Mutex {
 public:
  void Lock() {
    // Your code goes here
  }

  void Unlock() {
    // Your code goes here
  }

 private:
};

}  // namespace solutions
