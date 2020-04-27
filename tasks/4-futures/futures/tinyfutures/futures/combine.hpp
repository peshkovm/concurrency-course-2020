#pragma once

#include <tinyfutures/futures/promise.hpp>

#include <vector>

namespace tiny::futures {

namespace detail {

// Your combiners

}  // namespace detail

// All values / first error
template <typename T>
Future<std::vector<T>> All(std::vector<Future<T>> futures) {
  // Not implemented
  return MakeBrokenFuture<std::vector<T>>();
}

// First value or last error
template <typename T>
Future<T> FirstOf(std::vector<Future<T>> futures) {
  // Not implemented
  return MakeBrokenFuture<T>();
}

}  // namespace tiny::futures
