#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

.global FUNCTION_NAME(SwitchContext)

# SwitchContext(from, to)

FUNCTION_NAME(SwitchContext):
    # SwitchContext frame created on top of the current stack

    # 1. Save current execution context to 'from'

    # 1.1 Save callee-saved registers on top of the current stack

    # https://stackoverflow.com/questions/18024672/what-registers-are-preserved-through-a-linux-x86-64-function-call
    # https://uclibc.org/docs/psABI-x86_64.pdf

    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12

    pushq %rbx
    pushq %rbp

    # Switch stacks

    # 1.2 Save current stack pointer to 'from' ExecutionContext
    movq %rsp, (%rdi) # from->rsp_ := rsp

    # 2. Activate 'to' execution context

    # 2.1 Set stack pointer to target stack
    movq (%rsi), %rsp # rsp := to->rsp_

    # 2.2 Restore and pop registers saved on target stack

    popq %rbp
    popq %rbx

    popq %r12
    popq %r13
    popq %r14
    popq %r15

    # Pop current SwitchContext frame from target stack

    retq
