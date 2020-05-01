#include "after.hpp"

#include <tinyfutures/futures/promise.hpp>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/thread.hpp>

#include <asio.hpp>

#include <memory>

using namespace std::chrono_literals;

namespace tiny::futures {

using support::Duration;
using support::Unit;

namespace detail {

class TimeKeeper {
 public:
  TimeKeeper()
      : work_guard_(asio::make_work_guard(io_context_)),
        worker_thread_([this]() { Work(); }) {
  }

  ~TimeKeeper() {
    Stop();
  }

  Future<Unit> After(Duration d) {
    // asio requires copy-constructible handlers
    auto promise = std::make_shared<Promise<Unit>>();
    auto future = promise->MakeFuture();

    auto timer = std::make_shared<asio::steady_timer>(io_context_);
    timer->expires_after(d);
    auto complete_future = [timer, promise](std::error_code /*ignored*/) {
      std::move(*promise).SetValue(Unit{});
    };
    timer->async_wait(complete_future);
    return future;
  }

 private:
  void Work() {
    while (!stop_requested_.load()) {
      io_context_.run_for(1s);
    }
  }

  void Stop() {
    // TODO: cancel timers
    work_guard_.reset();
    stop_requested_.store(true);
    worker_thread_.join();
  }

 private:
  twist::stdlike::atomic<bool> stop_requested_{false};
  asio::io_context io_context_;
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  twist::stdlike::thread worker_thread_;
};

}  // namespace detail

Future<Unit> After(Duration d) {
  static detail::TimeKeeper time_keeper;
  return time_keeper.After(d);
}

}  // namespace tiny::futures
