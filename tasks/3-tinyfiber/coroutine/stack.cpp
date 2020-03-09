#include "stack.hpp"

#include <utility>

namespace tinyfiber {

static const size_t kStackPages = 8;  // 8KB stacks

Stack::Stack(MmapAllocation allocation) : allocation_(std::move(allocation)) {
}

Stack Stack::Allocate() {
  auto allocation = MmapAllocation::AllocatePages(kStackPages);
  allocation.ProtectPages(/*offset=*/0, /*count=*/1);
  return Stack{std::move(allocation)};
}

char* Stack::Bottom() const {
  return (char*)((std::uintptr_t*)allocation_.End() - 1);
}

MemSpan Stack::AsMemSpan() const {
  return allocation_.AsMemSpan();
}

}  // namespace tinyfiber
