#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>

namespace tiny::executors {

// Unbounded blocking queue

template <typename T>
class BlockingQueue {
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

 private:
  // Your code goes here
};

}  // namespace tiny::executors
