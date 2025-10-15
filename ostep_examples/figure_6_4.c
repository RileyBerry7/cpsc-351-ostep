// figure_6_4.c

// Demonstration of a simple context switch using swtch()
// Works together with figure_6_4.asm (x86-64 NASM)

// Build / Run Steps
// nasm -f elf64 figure_6_4.asm -o figure_6_4.o
// gcc figure_6_4.c figure_6_4.o -o figure_6_4

#include <stdio.h>
#include <stdint.h>

// -------------------------------------------------------------
// This structure must match what figure_6_4.asm saves/restores.
// Only callee-saved registers are switched (System V ABI).
// -------------------------------------------------------------
struct context {
    uint64_t rsp;   // Stack pointer
    uint64_t rbx;   // Callee-saved registers
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};

// The assembly routine provided in figure_6_4.asm
extern void swtch(struct context *old, struct context *new);

// Two separate register/stack contexts
static struct context main_ctx;
static struct context thread_ctx;

// A private stack for our fake "thread"
static uint8_t thread_stack[1024];

// -------------------------------------------------------------
// The function our "thread" will run once switched to.
// -------------------------------------------------------------
void thread_func(void) {
    printf("→ Now running inside thread_func\n");

    // Switch back to main (saves current, restores main)
    swtch(&thread_ctx, &main_ctx);

    // If control ever returns here (it won’t), print message
    printf("→ Returned to thread_func (unexpected)\n");
}

// -------------------------------------------------------------
// Program entry point
// -------------------------------------------------------------
int main(void) {
    printf("Starting in main()\n");

    // ---------------------------------------------------------
    // Prepare thread_ctx so that when swtch() returns into it,
    // the CPU "ret" will jump to thread_func().
    // ---------------------------------------------------------
    uint64_t *sp = (uint64_t *)(thread_stack + sizeof(thread_stack));
    *(--sp) = (uint64_t)thread_func;   // Fake return address for "ret"
    thread_ctx.rsp = (uint64_t)sp;     // Set stack pointer for thread

    // ---------------------------------------------------------
    // Save current registers into main_ctx and load thread_ctx.
    // This transfers control to thread_func().
    // ---------------------------------------------------------
    swtch(&main_ctx, &thread_ctx);

    // When thread_func calls swtch() back, execution resumes here.
    printf("Back in main() after returning from thread_func\n");

    return 0;
}

