#include "toyalloc.hpp"

#include <twist/memory/mmap_allocation.hpp>

#include <twist/test_framework/test_framework.hpp>

#include <algorithm>

TEST_SUITE(ToyAlloc) {
  SIMPLE_TEST(AllocateThenFree) {
    void* addr = toyalloc::Allocate();
    ASSERT_TRUE(addr != nullptr);
    toyalloc::Free(addr);
  }

  SIMPLE_TEST(AllocateAllArenaTwice) {
    auto arena = toyalloc::GetArena();
    size_t block_size = toyalloc::GetBlockSize();

    size_t block_count = arena.Size() / block_size;

    std::vector<void*> allocated;
    for (size_t i = 0; i < block_count; ++i) {
      void* addr = toyalloc::Allocate();
      ASSERT_TRUE(addr != nullptr);
      allocated.push_back(addr);
    }

    // Arena exhausted
    ASSERT_EQ(toyalloc::Allocate(), nullptr);

    // Free and allocate again
    void* first = allocated[0];
    toyalloc::Free(first);
    allocated[0] = toyalloc::Allocate();
    ASSERT_EQ(allocated[0], first);

    // Arena exhausted
    ASSERT_EQ(toyalloc::Allocate(), nullptr);

    // Release 1/2 blocks
    for (size_t i = 1; i < allocated.size(); i += 2) {
      toyalloc::Free(allocated[i]);
    }

    // Allocate again
    for (size_t i = 1; i < allocated.size(); i += 2) {
      allocated[i] = toyalloc::Allocate();
    }

    std::sort(allocated.begin(), allocated.end());

    for (size_t i = 0; i < allocated.size(); ++i) {
      char* start = arena.Begin() + i * block_size;
      ASSERT_EQ(start, allocated[i]);
    }
  }
}

void InitAllocator() {
  static const size_t kArenaPages = 1024;
  auto arena = twist::MmapAllocation::AllocatePages(kArenaPages);
  toyalloc::Init(std::move(arena));
}

int main() {
  InitAllocator();
  RunTests(ListAllTests());
}
