#pragma once

#include <tinyfutures/futures/future.hpp>
#include <tinysupport/result.hpp>

#include <memory>

namespace tiny::futures {

template <typename T>
class Future;

// Just for solution template =)
template <typename T>
Future<T> MakeBrokenFuture() {
  auto state = std::make_shared<detail::State<T>>();

  auto result = support::make_result::Invoke(
      []() -> T { throw std::runtime_error("Broken future"); });

  state->SetResult(std::move(result));
  return {std::move(state)};
}

}  // namespace tiny::futures
