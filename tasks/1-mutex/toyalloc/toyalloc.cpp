#include "toyalloc.hpp"

#include <twist/stdlike/atomic.hpp>
#include <twist/support/compiler.hpp>

#include <pthread.h>

using twist::MemSpan;
using twist::MmapAllocation;

namespace toyalloc {

static const size_t kBlockSize = 4096;

struct BlockNode {
  BlockNode* next_;
};

class Allocator {
 public:
  void Init(MmapAllocation arena) {
    arena_ = std::move(arena);
    // Your code goes here
  }

  MemSpan GetArena() const {
    return arena_.AsMemSpan();
  }

  void* Allocate() {
    // Your code goes here
    return nullptr;
  }

  void Free(void* addr) {
    // Your code goes here
    TWIST_UNUSED(addr);
  }

 private:
  MmapAllocation arena_;
  // Free list goes here
};

/////////////////////////////////////////////////////////////////////

static Allocator allocator;

/////////////////////////////////////////////////////////////////////

void Init(MmapAllocation arena) {
  allocator.Init(std::move(arena));
  // Prepare to fork?
}

MemSpan GetArena() {
  return allocator.GetArena();
}

size_t GetBlockSize() {
  return kBlockSize;
}

void* Allocate() {
  return allocator.Allocate();
}

void Free(void* addr) {
  allocator.Free(addr);
}

}  // namespace toyalloc
