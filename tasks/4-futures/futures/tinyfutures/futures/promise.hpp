#pragma once

#include <tinyfutures/futures/state.hpp>
#include <tinyfutures/futures/future.hpp>

#include <tinysupport/result.hpp>
#include <tinysupport/unit.hpp>

#include <memory>

namespace tiny::futures {

using support::Error;

using support::Result;
using support::make_result::Fail;
using support::make_result::Ok;

using support::Unit;

template <typename T>
class Promise : public detail::HoldState<T> {
  using detail::HoldState<T>::state_;
  using detail::HoldState<T>::CheckState;
  using detail::HoldState<T>::ReleaseState;

 public:
  Promise() : detail::HoldState<T>(std::make_shared<detail::State<T>>()) {
  }

  // One-shot
  Future<T> MakeFuture() {
    TINY_VERIFY(!future_extracted_, "Future already extracted");
    future_extracted_ = true;
    return Future{state_};
  }

  void SetValue(T value) && {
    ReleaseState()->SetResult(Ok(std::move(value)));
  }

  void SetError(Error error) && {
    ReleaseState()->SetResult(Fail(std::move(error)));
  }

  void Set(Result<T> result) && {
    ReleaseState()->SetResult(std::move(result));
  }

 private:
  bool future_extracted_{false};
};

//////////////////////////////////////////////////////////////////////

template <typename T>
using Contract = std::pair<Future<T>, Promise<T>>;

// Usage:
// auto [f, p] = futures::MakeContract<T>();
// https://en.cppreference.com/w/cpp/language/structured_binding

template <typename T>
Contract<T> MakeContract() {
  Promise<T> p;
  auto f = p.MakeFuture();
  return {std::move(f), std::move(p)};
}

}  // namespace tiny::futures
