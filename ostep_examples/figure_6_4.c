// figure_6_4.c

// Demonstration of a simple context switch using swtch()
// Works together with figure_6_4.asm (x86-64 NASM)

// Build / Run Steps
// nasm -f elf64 figure_6_4.asm -o figure_6_4.o
// gcc figure_6_4.c figure_6_4.o -o figure_6_4

#include <stdio.h>
#include <stdint.h>

// -------------------------------------------------------------------------------------------
// Struct: Context - contains process 'context' (GPRs, Stack Ptr, Program Counter)
// -------------------------------------------------------------------------------------------
struct context {

    uint64_t rsp;   // Stack pointer
    uint64_t rbx;   // Callee-saved registers
    uint64_t rbp;
    uint64_t r12;   // uint64_t = fixed-width 64-bit integers
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};


// Declares assembly routine from .asm
extern void context_switch(struct context *old, struct context *new);

static struct context curr_ctx;    // old context
static struct context next_ctx;    // new context
static uint8_t next_stack[1024]; // private stack for 'process 2' 

void process_2(void) {
    
    printf("\nProcess 2:\n");
    printf(" ...\n");

    // Switch back to main (saves current, restores main)
    context_switch(&next_ctx, &curr_ctx);

    // ---- Unreachable ----
    printf("[UNEXPECTED]\n");
}

// -------------------------------------------------------------
// Program entry point
// -------------------------------------------------------------
int main(void) {
    
    uint64_t *stack_ptr;

    printf("\nProcess 1:\n");
    printf("  setting up stack p2 stack\n");

    stack_ptr = (uint64_t *)(next_stack + sizeof(next_stack)); 
    *(--stack_ptr) = (uint64_t)process_2;   // Fake return address for "ret"
    next_ctx.rsp = (uint64_t)stack_ptr;     // Set stack pointer for thread
    
    printf("  performing context switch\n");
    context_switch(&curr_ctx, &next_ctx); // swap contexts (transfers control to thread_func)
    
    // When thread_func calls swtch() back, execution resumes here.
    printf("\nProcess 1:\n");
    printf("  ...\n");

    return 0;
}

