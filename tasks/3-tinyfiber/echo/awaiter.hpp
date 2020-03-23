#pragma once

#include <tinysupport/result.hpp>

#include "scheduler.hpp"

namespace tinyfiber {

template <typename T>
class Awaiter {
 public:
  Result<T> Await();
};

template <>
class Awaiter<void> {
 public:
  Status Await();
};

}  // namespace tinyfiber
