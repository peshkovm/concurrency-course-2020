#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

namespace solutions {

class Semaphore {
 public:
  explicit Semaphore(size_t initial_count) {
    // Not implemented
    count_of_tickets_ = initial_count;
  }

  void Acquire() {
    // Not implemented
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (count_of_tickets_ == 0) {
      semaphore_not_empty_.wait(lock);
    }
    count_of_tickets_--;
  }

  void Release() {
    // Not implemented
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    count_of_tickets_++;
    semaphore_not_empty_.notify_one();
  }

 private:
  // Your code goes here
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable semaphore_not_empty_;
  size_t count_of_tickets_;
};

}  // namespace solutions
