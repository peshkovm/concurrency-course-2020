#pragma once

#include "semaphore.hpp"

#include <deque>
#include <iostream>

namespace solutions {

template <typename T>
class BufferedChannel {
 public:
  explicit BufferedChannel(size_t capacity) : queue_is_full_(capacity) {
    // Not implemented
  }

  void Send(T item) {
    queue_is_full_.Acquire();
    mutex_.Acquire();
    deque_.push_back(std::move(item));
    queue_is_empty_.Release();
    mutex_.Release();
  }

  T Receive() {
    // Not implemented
    queue_is_empty_.Acquire();
    mutex_.Acquire();
    auto item = std::move(deque_.front());
    deque_.pop_front();
    queue_is_full_.Release();
    mutex_.Release();

    return item;
  }

 private:
  // Use only semaphores for synchronization
  solutions::Semaphore queue_is_full_;
  solutions::Semaphore queue_is_empty_{0};
  solutions::Semaphore mutex_{1};
  std::deque<T> deque_;
};

}  // namespace solutions
