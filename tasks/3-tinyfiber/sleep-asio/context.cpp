#include "context.hpp"
#include "stack.hpp"

#include <cstdint>

namespace tinyfiber {

// Switch between ExecutionContext-s
extern "C" void SwitchContext(ExecutionContext* from, ExecutionContext* to);

// View for stack-saved context
struct StackSavedContext {
  // Layout of the StackSavedContext matches the layout of the stack
  // in context.S at the 'Switch stacks' comment

  // Callee-saved registers
  // Saved manually in SwitchContext
  void* rbp;
  void* rbx;

  void* r12;
  void* r13;
  void* r14;
  void* r15;

  // Saved automatically by 'call' instruction
  void* rip;
};

void ExecutionContext::Setup(MemSpan stack, Trampoline trampoline) {
  // https://eli.thegreenplace.net/2011/02/04/where-the-top-of-the-stack-is-on-x86/

  StackBuilder builder(stack.Back());

  // Ensure trampoline will get 16-byte aligned frame pointer (rbp)
  // 'Next' here means first 'pushq %rbp' in trampoline prologue
  builder.AlignNextPush(16);

  // Reserve space for stack-saved context
  builder.Allocate(sizeof(StackSavedContext));

  auto* saved_context = (StackSavedContext*)builder.Top();
  saved_context->rip = (void*)trampoline;

  // Set current stack top
  rsp_ = saved_context;
}

static size_t switch_count = 0;

void ExecutionContext::SwitchTo(ExecutionContext& target) {
  ++switch_count;
  SwitchContext(this, &target);
}

// For testing purposes
size_t ExecutionContext::SwitchCount() {
  return switch_count;
}

}  // namespace tinyfiber
