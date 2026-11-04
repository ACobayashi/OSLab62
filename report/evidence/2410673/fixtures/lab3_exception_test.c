#include <assert.h>
#include <defs.h>
#include <riscv.h>
#include <sbi.h>
#include <stdio.h>
#include <trap.h>

#define PROBE(name) extern int name(void); extern char name##_site[], name##_resume[]
PROBE(lab3_illegal32);
PROBE(lab3_illegal16);
PROBE(lab3_breakpoint16);
PROBE(lab3_breakpoint32);
extern uintptr_t hyc_edit_probe(void);
extern char hyc_edit_site[], hyc_edit_resume[];

struct test_case { int (*run)(void); char *site; char *resume; uintptr_t cause; };
static const struct test_case cases[] = {
    {lab3_illegal32, lab3_illegal32_site, lab3_illegal32_resume, CAUSE_ILLEGAL_INSTRUCTION},
    {lab3_illegal16, lab3_illegal16_site, lab3_illegal16_resume, CAUSE_ILLEGAL_INSTRUCTION},
    {lab3_breakpoint16, lab3_breakpoint16_site, lab3_breakpoint16_resume, CAUSE_BREAKPOINT},
    {lab3_breakpoint32, lab3_breakpoint32_site, lab3_breakpoint32_resume, CAUSE_BREAKPOINT},
};
static const struct test_case *active;
static uintptr_t expected_gp, expected_tp, expected_sie;
static unsigned int entries, returns;
static bool edit;

/* Observe real entry frames before the real exception handler executes. */
void hyc_check_frame(struct trapframe *tf) {
    uintptr_t *regs = (uintptr_t *)&tf->gpr;
    assert(sizeof(struct pushregs) == 32 * sizeof(uintptr_t));
    assert(sizeof(struct trapframe) == 36 * sizeof(uintptr_t));
    assert((uintptr_t)&tf->status - (uintptr_t)tf == 32 * sizeof(uintptr_t));
    assert((uintptr_t)&tf->epc - (uintptr_t)tf == 33 * sizeof(uintptr_t));
    assert(((uintptr_t)tf & 15) == 0);
    assert((uintptr_t)tf + sizeof(*tf) == tf->gpr.sp);
    assert(tf->gpr.zero == 0);
    assert(read_csr(sscratch) == 0);
    assert((read_csr(sstatus) & SSTATUS_SIE) == 0);
    assert((tf->status & SSTATUS_SPP) != 0);
    assert((tf->status & SSTATUS_SIE) == 0);
    assert(((tf->status & SSTATUS_SPIE) != 0) == (expected_sie != 0));
    if (edit) {
        assert(tf->epc == (uintptr_t)hyc_edit_site);
        assert(tf->cause == CAUSE_BREAKPOINT);
    } else {
        assert(tf->epc == (uintptr_t)active->site);
        assert(tf->cause == active->cause);
        assert(tf->gpr.ra == *(uintptr_t *)(tf->gpr.sp + 96));
        assert(tf->gpr.gp == expected_gp && tf->gpr.tp == expected_tp);
        for (size_t i = 5; i < 32; i++) assert(regs[i] == 0x100 + i);
    }
    entries++;
}

/* Confirm sepc follows the C handler; exercise its deliberate frame edit too. */
void hyc_check_return(struct trapframe *tf) {
    assert(tf->epc == (uintptr_t)(edit ? hyc_edit_resume : active->resume));
    if (edit) tf->gpr.a0 = 0x2410673;
    returns++;
}

void lab3_test_exceptions(void) {
    /* Keep timer sources masked while testing the global SIE state itself. */
    write_csr(sie, 0);
    asm volatile("mv %0, gp\nmv %1, tp" : "=r"(expected_gp), "=r"(expected_tp));
    for (unsigned int enabled = 0; enabled < 2; enabled++) {
        expected_sie = enabled ? SSTATUS_SIE : 0;
        if (enabled) set_csr(sstatus, SSTATUS_SIE);
        else clear_csr(sstatus, SSTATUS_SIE);
        edit = 0;
        for (unsigned int repeat = 0; repeat < 32; repeat++) {
            for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
                active = &cases[i];
                assert(active->run() == 0);
                assert((read_csr(sstatus) & SSTATUS_SIE) == expected_sie);
                assert((read_csr(sstatus) & SSTATUS_SPP) == 0);
                assert((read_csr(sstatus) & SSTATUS_SPIE) != 0);
                assert(read_csr(sscratch) == 0);
            }
        }
        edit = 1;
        assert(hyc_edit_probe() == 0x2410673);
        assert((read_csr(sstatus) & SSTATUS_SIE) == expected_sie);
        clear_csr(sstatus, SSTATUS_SIE);
        cprintf("[HYC] SIE=%d: 128 real traps, all GPR/frame/stack checks, EPC and edited a0: PASS\n", enabled);
    }
    assert(entries == 258 && returns == 258);
    cprintf("[HYC] 258 entry/return cycles: PASS\n");
    sbi_shutdown();
    panic("shutdown returned");
}
