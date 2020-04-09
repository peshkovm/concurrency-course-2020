#pragma once

#include "semaphore.hpp"

#include <deque>

namespace solutions {

template <typename T>
class BufferedChannel {
 public:
  explicit BufferedChannel(size_t /*capacity*/) {
    // Not implemented
  }

  void Send(T /*item*/) {
    // Not implemented
  }

  T Receive() {
    // Not implemented
  }

 private:
  // Use only semaphores for synchronization
};

}  // namespace solutions
