#include <assert.h>
#include <trap.h>

/* Test fixture only: exception tests run before clock_init/intr_enable. */
void interrupt_handler(struct trapframe *tf) {
    print_trapframe(tf);
    panic("unexpected interrupt in exception-only test");
}
