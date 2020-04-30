#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>

namespace tiny::executors {

// Multi-producer/multi-consumer (MPMC) unbounded blocking queue

template <typename T>
class MPMCBlockingQueue {
 public:
  // Returns false iff queue is closed / shutted down
  bool Put(T item) {
    return false;  // Not implemented
  }

  // Await and dequeue next item
  // Returns false iff queue is both 1) drained and 2) closed
  std::optional<T> Take() {
    return {};  // Not implemented
  }

  // Close queue for producers
  void Close() {
    // Not implemented
  }

  // Close queue for producers and consumers,
  // discard existing items
  void Shutdown() {
    // Not implemented
  }
};

//////////////////////////////////////////////////////////////////////

// Multi-producer/single-consumer (MPSC) unbounded lock-free queue

template <typename T>
class MPSCLockFreeQueue {
 public:
  void Put(T item) {
    // Not implemented
  }

  // Take? or TakeAll?
};

}  // namespace tiny::executors
