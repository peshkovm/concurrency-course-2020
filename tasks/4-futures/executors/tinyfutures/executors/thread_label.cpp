#include "thread_label.hpp"

#include <tinysupport/assert.hpp>

namespace tiny::executors {

static thread_local ThreadLabel thread_label;

void LabelThread(const ThreadLabel& label) {
  thread_label = label;
}

void ExpectThread(const ThreadLabel& label) {
  TINY_VERIFY(label == thread_label, "Unexpected thread label: '"
                                         << thread_label << "', expected `"
                                         << label << "`");
}

ThreadLabel GetThreadLabel() {
  return thread_label;
}

}  // namespace tiny::executors
