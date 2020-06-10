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
    ##########PROLOGUE##########
    pushq   %rbp
    movq    %rsp, %rbp
    ##########PROLOGUE##########

    movq %rsi, (%rdi)

    ##########EPILOGUE##########
    mov %rbp, %rsp
    popq %rbp
    ##########EPILOGUE##########
    retq

FUNCTION_NAME(AtomicExchange):
    # Your asm code goes here
    ##########PROLOGUE##########
    pushq   %rbp
    movq    %rsp, %rbp
    ##########PROLOGUE##########

    xchgq (%rdi), %rsi
    movq %rsi, %rax

    ##########EPILOGUE##########
    mov %rbp, %rsp
    popq %rbp
    ##########EPILOGUE##########
    retq
