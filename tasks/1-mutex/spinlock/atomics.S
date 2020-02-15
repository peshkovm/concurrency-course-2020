#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.globl FUNCTION_NAME(AtomicStore)
.globl FUNCTION_NAME(AtomicExchange)

# Solution starts here

FUNCTION_NAME(AtomicStore):
    # Your asm code goes here
    movq %rsi, (%rdi)
    retq

FUNCTION_NAME(AtomicExchange):
    # Your asm code goes here
    movq (%rdi), %rax
    movq %rsi, (%rdi)
    retq
