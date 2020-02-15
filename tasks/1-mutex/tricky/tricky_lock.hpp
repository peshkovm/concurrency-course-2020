#pragma once

#include <atomic>

namespace solutions {

class TrickyLock {
 public:
  void Lock() {
    while (thread_count_.fetch_add(1) > 0) {
      thread_count_.fetch_sub(1);
    }
  }

  void Unlock() {
    thread_count_.fetch_sub(1);
  }

 private:
  std::atomic<size_t> thread_count_{0};
};

}  // namespace solutions
