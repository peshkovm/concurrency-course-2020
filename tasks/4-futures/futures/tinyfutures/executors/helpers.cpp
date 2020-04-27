#include <tinyfutures/executors/helpers.hpp>

namespace tiny::executors {

void SafelyExecuteHere(Task& task) {
  try {
    task();
  } catch (...) {
    // ¯\_(ツ)_/¯
  }
}

}  // namespace tiny::executors
