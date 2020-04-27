#pragma once

#include <tinyfutures/futures/future.hpp>

#include <tinysupport/unit.hpp>
#include <tinysupport/time.hpp>

namespace tiny::futures {

Future<support::Unit> After(support::Duration d);

}  // namespace tiny::futures
