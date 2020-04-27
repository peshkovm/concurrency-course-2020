#pragma once

#include <tinyfutures/futures/state.hpp>
#include <tinyfutures/futures/helpers.hpp>

namespace tiny::futures {

template <typename T>
class Promise;

using executors::IExecutorPtr;

template <typename T>
class Future : public detail::HoldState<T> {
  friend class Promise<T>;

  using detail::HoldState<T>::state_;
  using detail::HoldState<T>::HasState;
  using detail::HoldState<T>::CheckState;
  using detail::HoldState<T>::ReleaseState;

  // Subscribe
  // Result<T> -> void
  using Callback = typename detail::State<T>::Callback;

  // Then
  // Result<T> -> U
  template <typename U>
  using Continuation = support::UniqueFunction<U(Result<T>)>;

  template <typename U>
  friend Future<U> MakeBrokenFuture();

 public:
  bool IsValid() const {
    return HasState();
  }

  Result<T> GetResult() && {
    return ReleaseState()->GetResult();
  }

  T GetValue() && {
    return ReleaseState()->GetResult().Value();
  }

  Future<T> Via(executors::IExecutorPtr e) && {
    // Not implemented
    return MakeBrokenFuture<T>();
  }

  void Subscribe(Callback callback) && {
    // Not implemented
  }

  // Support arbitrary callable-s
  template <typename F>
  auto Then(F&& f) && {
    using U = decltype(f(std::declval<Result<T>>()));

    return std::move(*this).Then(Continuation<U>(std::forward<F>(f)));
  }

  // Synchronous continuation

  template <typename U>
  Future<U> Then(Continuation<U> cont) && {
    // Not implemented
    return MakeBrokenFuture<U>();
  }

  // Asynchronous continuation

  template <typename U>
  Future<U> Then(Continuation<Future<U>> f) && {
    // Not implemented
    return MakeBrokenFuture<U>();
  }

 private:
  Future(detail::StateRef<T> state) : detail::HoldState<T>(std::move(state)) {
  }
};

}  // namespace tiny::futures
