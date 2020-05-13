#include "thread_label.hpp"

#include <tinysupport/assert.hpp>
#include <tinysupport/string.hpp>

#include <twist/strand/thread_local.hpp>

namespace tiny::executors {

using tiny::support::Quoted;

// Fibers execution backend support: thread_local -> ThreadLocal
static twist::strand::ThreadLocal<ThreadLabel> thread_label;

void LabelThread(const ThreadLabel& label) {
  *thread_label = label;
}

void ExpectThread(const ThreadLabel& label) {
  TINY_VERIFY(label == *thread_label,
              "Unexpected thread label: " << Quoted(*thread_label)
                                          << ", expected " << Quoted(label));
}

ThreadLabel GetThreadLabel() {
  return *thread_label;
}

}  // namespace tiny::executors
