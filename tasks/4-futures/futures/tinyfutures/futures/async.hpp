#pragma once

#include <tinyfutures/executors/executor.hpp>
#include <tinyfutures/futures/promise.hpp>
#include <tinysupport/result.hpp>

namespace tiny::futures {

// Execute callable object `f` via executor `e`
// and return future
//
// Usage:
// auto tp = MakeStaticThreadPool(4, "tp");
// std::cout << AsyncVia([]() { return 42; }, tp).GetValue() << std;:endl;

template <typename F>
auto AsyncVia(F&& f, executors::IExecutorPtr e) {
  using T = decltype(f());

  // Not implemented
  return MakeBrokenFuture<T>();
}

}  // namespace tiny::futures
