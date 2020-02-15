#pragma once

#include <cstdint>

// x86-64 only!

// Atomically stores 'value' to memory location 'addr'
extern "C" void AtomicStore(volatile std::int64_t* addr, std::int64_t value);

// Atomically replaces content of memory location `addr` with `value`,
// returns content of the location before the call
extern "C" int AtomicExchange(volatile std::int64_t* addr, std::int64_t value);
