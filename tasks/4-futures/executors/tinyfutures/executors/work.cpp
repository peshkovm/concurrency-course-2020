#include <tinyfutures/executors/work.hpp>

namespace tiny::executors {

WorkGuard MakeWorkFor(IExecutorPtr e) {
  return {std::move(e)};
}

}  // namespace tiny::executors
