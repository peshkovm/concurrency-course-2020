#include "dining.hpp"

#include <twist/test_framework/test_framework.hpp>
#include <twist/fault/adversary/inject_fault.hpp>

namespace dining {

void Plate::Access() {
  ASSERT_FALSE_M(accessed_.exchange(true, std::memory_order_relaxed),
                 "Mutual exclusion violated");
  twist::fault::InjectFault();
  ASSERT_TRUE_M(accessed_.exchange(false, std::memory_order_relaxed),
                "Mutual exclusion violated");
}

}  // namespace dining
