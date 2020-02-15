#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/strand/spin_wait.hpp>

namespace solutions {

using twist::strand::SpinWait;

class TicketLock {
 public:
  // Don't change this method
  void Lock() {
    const size_t this_thread_ticket = next_free_ticket_.fetch_add(1);

    SpinWait spin_wait;
    while (this_thread_ticket != owner_ticket_.load()) {
      spin_wait();
    }
  }

  bool TryLock() {
    return false;  // To be implemented
  }

  // Don't change this method
  void Unlock() {
    owner_ticket_.store(owner_ticket_.load() + 1);
  }

 private:
  twist::stdlike::atomic<size_t> next_free_ticket_{0};
  twist::stdlike::atomic<size_t> owner_ticket_{0};
};

}  // namespace solutions
