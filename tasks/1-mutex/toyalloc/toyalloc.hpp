#pragma once

#include <twist/memory/mmap_allocation.hpp>

namespace toyalloc {

void Init(twist::MmapAllocation arena);

// Returns arena memory span
twist::MemSpan GetArena();

size_t GetBlockSize();

// Allocates block
void* Allocate();

// Releases previously allocated block
void Free(void* addr);

}  // namespace toyalloc
