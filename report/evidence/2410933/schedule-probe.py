#!/usr/bin/env python3
# LAB4 function3 2410933
from pathlib import Path

p = Path('kern/init/init.c')
s = p.read_text()
anchor = '    cpu_idle();'
assert s.count(anchor) == 1
s = s.replace('#include <dtb.h>', '#include <dtb.h>\n#include <sched.h>\n#include <sync.h>\n#include <assert.h>\n#include <sbi.h>')
body = r'''
// LAB4 function3 2410933
extern int probe_switch(void);
static volatile int worker_ran;

static int sie(void)
{
    return !!(read_csr(sstatus) & SSTATUS_SIE);
}

static int probe_worker(void *arg)
{
    assert(current != idleproc && sie() == 1);
    for (int enabled = 0; enabled <= 1; enabled++)
    {
        if (enabled) intr_enable();
        else intr_disable();
        struct proc_struct *self = current;
        int runs = self->runs;
        self->need_resched = 1;
        schedule();
        assert(current == self && self->runs == runs + 1);
        assert(!self->need_resched && sie() == enabled);
    }
    cprintf("[2410933] sole runnable thread and both SIE states PASS\n");
    worker_ran++;
    current->state = PROC_SLEEPING;
    schedule();
    panic("sleeping test worker must not resume");
}

static void probe_schedule(void)
{
    assert(current == idleproc && sie() == 1);
    list_del_init(&initproc->list_link);
    int runs = idleproc->runs;
    schedule();
    assert(current == idleproc && idleproc->runs == runs + 1 && list_empty(&proc_list));
    cprintf("[2410933] empty process list PASS\n");

    initproc->state = PROC_SLEEPING;
    list_add(&proc_list, &initproc->list_link);
    runs = idleproc->runs;
    schedule();
    assert(current == idleproc && idleproc->runs == runs + 1 && initproc->runs == 0);
    cprintf("[2410933] all sleeping: idle fallback PASS\n");

    for (int enabled = 0; enabled <= 1; enabled++)
    {
        int pid = kernel_thread(probe_worker, NULL, 0);
        assert(pid > 0);
        struct proc_struct *worker = find_proc(pid);
        list_del_init(&initproc->list_link);
        list_add(&proc_list, &initproc->list_link);
        if (enabled) intr_enable();
        else intr_disable();

        assert(probe_switch() == 0);
        assert(current == idleproc && worker_ran == enabled + 1 && worker->runs == 3);
        assert(initproc->runs == 0 && sie() == enabled);
        cprintf("[2410933] skip sleeper, switch back, registers/stack, initial SIE=%d PASS\n", enabled);
    }

    bool outer, inner;
    local_intr_save(outer);
    assert(outer && !sie());
    local_intr_save(inner);
    assert(!inner && !sie());
    local_intr_restore(inner);
    assert(!sie());
    local_intr_restore(outer);
    assert(sie());
    cprintf("[2410933] nested interrupt save/restore PASS\n");
    cprintf("[2410933] ALL PASS\n");
    sbi_shutdown();
}

'''
s = s.replace('int kern_init(void)\n', body + 'int kern_init(void)\n',1)
p.write_text(s.replace(anchor, '    probe_schedule();\n'+anchor))
regs = [f's{i}' for i in range(12)]
asm = ['# LAB4 function3 2410933', '.text', '.globl probe_switch', 'probe_switch:', '    addi sp, sp, -128', '    sd ra, 0(sp)']
asm += [f'    sd {r}, {8*(i+1)}(sp)' for i,r in enumerate(regs)]
asm += [f'    li {r}, {111+i}' for i,r in enumerate(regs[:-1])]
asm += ['    mv s11, sp', '    li t0, 0x2410933', '    sd t0, 104(sp)', '    call schedule', '    bne sp, s11, .Lbad']
for i,r in enumerate(regs[:-1]):
    asm += [f'    li t0, {111+i}', f'    bne {r}, t0, .Lbad']
asm += ['    ld t0, 104(sp)', '    li t1, 0x2410933', '    bne t0, t1, .Lbad', '    li a0, 0', '    j .Lrestore', '.Lbad:', '    li a0, 1', '.Lrestore:']
asm += [f'    ld {r}, {8*(i+1)}(sp)' for i,r in enumerate(regs)]
asm += ['    ld ra, 0(sp)', '    addi sp, sp, 128', '    ret']
Path('kern/init/lab4_probe.S').write_text('\n'.join(asm)+'\n')
