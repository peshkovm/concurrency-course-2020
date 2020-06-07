#if (APPLE)
  #define FUNCTION_NAME(name) _##name
#else
  #define FUNCTION_NAME(name) name
#endif

# Make symbols visible for linker
.globl FUNCTION_NAME(Capture)
.globl FUNCTION_NAME(JumpTo)

# Solution starts here
# You need just two instructions: 'movq' and 'ret'

FUNCTION_NAME(Capture):
    ##########PROLOGUE##########
    pushq   %rbp
    movq    %rsp, %rbp
    movq    %rdi, -8(%rbp)
    ##########PROLOGUE##########
    movq    -8(%rbp), %rdi //ctx
    movq    8(%rbp), %rax //rip on stack
    movq    %rax, (%rdi) //ctx->rip = rip on stack
    ##########EPILOGUE##########
    mov %rbp, %rsp
    popq %rbp
    ##########EPILOGUE##########
    ret

FUNCTION_NAME(JumpTo):
    ##########PROLOGUE##########
    pushq   %rbp
    movq    %rsp, %rbp
    movq    %rdi, -8(%rbp)
    ##########PROLOGUE##########
    movq    -8(%rbp), %rdi //ctx
    movq    (%rdi), %rdx //ctx->rip
    movq    %rdx, 8(%rbp) //rip on stack = ctx->rip
    ##########EPILOGUE##########
    mov %rbp, %rsp
    popq %rbp
    ##########EPILOGUE##########
    ret
