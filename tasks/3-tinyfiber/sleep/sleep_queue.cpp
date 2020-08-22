#include "sleep_queue.hpp"

namespace tinyfiber {

// SleepFiberNode
SleepFiberNode::SleepFiberNode(Fiber* fiber, Duration duration)
    : fiber_(fiber), duration_(duration) {
}

Duration SleepFiberNode::RemainingSleepTime() const {
  return std::max(Duration(0), duration_ - stop_watch_.Elapsed());
}

Duration SleepFiberNode::RemainingSleepTimeFrom(TimePoint from_time) const {
  return std::max(Duration(0), duration_ - stop_watch_.ElapsedFrom(from_time));
}

bool SleepFiberNode::IsReadyToWakedUp() const {
  return duration_ <= stop_watch_.Elapsed();
}

Fiber* SleepFiberNode::GetFiber() const {
  return fiber_;
}

// min tree by sleep time
bool SleepFiberNode::operator<(const SleepFiberNode& other) const {
  auto now = MyStopWatch::Now();
  return this->RemainingSleepTimeFrom(now) > other.RemainingSleepTimeFrom(now);
}

// SleepQueue
void SleepQueue::PutFiberSleepFor(Fiber* fiber, Duration duration) {
  push(SleepFiberNode(std::move(fiber), std::move(duration)));
}

bool SleepQueue::IsEmpty() const {
  return empty();
}

Fiber* SleepQueue::TakeReadyToWakeUpFiber() {
  if (empty() || !top().IsReadyToWakedUp()) {
    return nullptr;
  }
  auto fiber = top().GetFiber();
  pop();
  return fiber;
}

Duration SleepQueue::MinSleepTime() {
  return empty() ? Duration(0) : top().RemainingSleepTime();
}

bool SleepQueue::IsAnyoneReadyToWakeUp() {
  return !empty() && top().IsReadyToWakedUp();
}

}  // namespace tinyfiber
