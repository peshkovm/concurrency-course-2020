#pragma once

#include <tinyfutures/futures/after.hpp>
#include <tinyfutures/futures/promise.hpp>

#include <twist/stdlike/atomic.hpp>

#include <exception>
#include <memory>

namespace tiny::futures {

struct TimedOut : public std::runtime_error {
  TimedOut() : std::runtime_error("Operation timed out") {
  }
};

namespace detail {

}  // namespace detail

// `f` or `TimeOut` exception
template <typename T>
Future<T> WithTimeout(Future<T> f, support::Duration timeout) {
  // Not implemented
  return MakeBrokenFuture<T>();
}

}  // namespace tiny::futures
