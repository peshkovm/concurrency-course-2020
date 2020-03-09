#pragma once

#include <asio.hpp>

namespace tinyfiber {

using WaitableTimer = asio::basic_waitable_timer<std::chrono::steady_clock>;

}  // namespace tinyfiber
