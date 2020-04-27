#pragma once

#include <tinyfutures/executors/executor.hpp>

#include <tinysupport/function.hpp>
#include <tinysupport/result.hpp>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>

namespace tiny::futures {

using executors::IExecutorPtr;
using support::Result;

namespace detail {

//////////////////////////////////////////////////////////////////////

// State shared between Promise and Future

template <typename T>
class State {
 public:
  using Callback = support::UniqueFunction<void(Result<T>)>;

 public:
  void SetResult(Result<T>&& result) {
    // Not implemented
    result_.emplace(std::move(result));
  }

  Result<T> GetResult() {
    // Not implemented
    return std::move(*result_);
  }

 private:
  std::optional<Result<T>> result_;
};

//////////////////////////////////////////////////////////////////////

// Common base for Promise and Future

template <typename T>
using StateRef = std::shared_ptr<State<T>>;

template <typename T>
class HoldState {
 protected:
  HoldState(StateRef<T> state) : state_(std::move(state)) {
  }

  // Movable
  HoldState(HoldState&& that) = default;
  HoldState& operator=(HoldState&& that) = default;

  // Non-copyable
  HoldState(const HoldState& that) = delete;
  HoldState& operator=(const HoldState& that) = delete;

  StateRef<T> ReleaseState() {
    CheckState();
    return std::move(state_);
  }

  bool HasState() const {
    return (bool)state_;
  }

  void CheckState() {
    TINY_VERIFY(state_, "No state");
  }

 protected:
  StateRef<T> state_;
};

}  // namespace detail

}  // namespace tiny::futures
