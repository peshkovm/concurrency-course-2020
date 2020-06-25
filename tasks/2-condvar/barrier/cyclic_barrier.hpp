#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <cstddef>

namespace solutions {

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t participants /*participants*/) {
    this->num_of_participants_ = participants;
    num_of_threads_to_arrive_ = participants;
  }

  void Arrive() {
    // Not implemented
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    int local_num_of_wave = num_of_wave_to_wait_;
    --num_of_threads_to_arrive_;

    while (num_of_threads_to_arrive_ > 0 &&
           local_num_of_wave == num_of_wave_to_wait_) {
      all_threads_arrived_.wait(lock);
    }
    if (num_of_threads_to_arrive_ == 0) {
      all_threads_arrived_.notify_all();
      num_of_threads_to_arrive_ = num_of_participants_;
      num_of_wave_to_wait_++;
    }
  }

 private:
  // Your code goes here
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable all_threads_arrived_;
  int num_of_threads_to_arrive_;
  int num_of_participants_;
  int num_of_wave_to_wait_ = 0;
};

}  // namespace solutions
