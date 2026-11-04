#include <assert.h>
#include <defs.h>
#include <lab3_exception_test.h>
#include <riscv.h>
#include <sbi.h>
#include <stdio.h>
#include <string.h>
#include <trap.h>

extern void exception_handler(struct trapframe *tf);

#define DECLARE_PROBE(name) \
    extern int name(void); \
    extern char name##_site[], name##_resume[]

DECLARE_PROBE(lab3_illegal32);
DECLARE_PROBE(lab3_illegal16);
DECLARE_PROBE(lab3_breakpoint16);
DECLARE_PROBE(lab3_breakpoint32);

struct exception_case {
    const char *name;
    int (*probe)(void);
    char *site;
    char *resume;
    uintptr_t cause;
    uintptr_t length;
};

static const struct exception_case cases[] = {
    {"illegal32", lab3_illegal32, lab3_illegal32_site, lab3_illegal32_resume,
     CAUSE_ILLEGAL_INSTRUCTION, 4},
    {"illegal16", lab3_illegal16, lab3_illegal16_site, lab3_illegal16_resume,
     CAUSE_ILLEGAL_INSTRUCTION, 2},
    {"breakpoint16", lab3_breakpoint16, lab3_breakpoint16_site,
     lab3_breakpoint16_resume, CAUSE_BREAKPOINT, 2},
    {"breakpoint32", lab3_breakpoint32, lab3_breakpoint32_site,
     lab3_breakpoint32_resume, CAUSE_BREAKPOINT, 4},
};

static void test_synthetic_frames(void) {
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const struct exception_case *test = &cases[i];
        struct trapframe tf, expected;
        memset(&tf, 0x5a, sizeof(tf));
        tf.cause = test->cause;
        tf.epc = (uintptr_t)test->site;
        expected = tf;
        expected.epc = (uintptr_t)test->resume;
        assert(expected.epc - tf.epc == test->length);
        exception_handler(&tf);
        assert(memcmp(&tf, &expected, sizeof(tf)) == 0);
        cprintf("[LZY synthetic] %s: EPC +%d, other fields unchanged: PASS\n",
                test->name, (int)test->length);
    }
}

static void test_fallbacks(void) {
    const uintptr_t causes[] = {CAUSE_FAULT_LOAD, 20};
    for (size_t i = 0; i < sizeof(causes) / sizeof(causes[0]); i++) {
        struct trapframe tf, expected;
        memset(&tf, 0, sizeof(tf));
        tf.cause = causes[i];
        /* EPC=0 must not be dereferenced or advanced in the default branch. */
        expected = tf;
        exception_handler(&tf);
        assert(memcmp(&tf, &expected, sizeof(tf)) == 0);
        cprintf("[LZY synthetic] fallback cause=%d, EPC unchanged: PASS\n",
                (int)tf.cause);
    }
}

void lab3_test_exceptions(void) {
    cprintf("[LZY] exception-only tests, student 2410936\n");
    cprintf("[LZY] reference entry/return may be used; this is NOT make grade\n");
    test_synthetic_frames();
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const struct exception_case *test = &cases[i];
        uintptr_t interrupt_state = read_csr(sstatus) & SSTATUS_SIE;
        cprintf("[LZY real] %s: cause=%d, length=%d, site=%p, resume=%p\n",
                test->name, (int)test->cause, (int)test->length,
                test->site, test->resume);
        assert(test->probe() == 0);
        assert((read_csr(sstatus) & SSTATUS_SIE) == interrupt_state);
        cprintf("[LZY real] %s: continued, GPR/stack return: PASS\n", test->name);
    }
    test_fallbacks();
    cprintf("[LZY] All exception tests: PASS\n");
    sbi_shutdown();
    panic("SBI shutdown returned");
}
