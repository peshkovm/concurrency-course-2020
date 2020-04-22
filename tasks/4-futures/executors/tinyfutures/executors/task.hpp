#pragma once

#include <tinysupport/function.hpp>

namespace tiny::executors {

// Move-only task
// Can wrap move-only lambdas
using Task = support::UniqueFunction<void()>;

}  // namespace tiny::executors
