#pragma once

#include <twist/stdlike/mutex.hpp>

namespace solutions {

// Automagically wraps all accesses to guarded object to critical sections
// Look at unit tests for API and usage examples
template <typename T>
class Guarded {
  // Your code goes here

 private:
  T object_;
  twist::stdlike::mutex mutex_;
};

}  // namespace solutions

