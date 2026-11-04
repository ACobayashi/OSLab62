set pagination off
set confirm off
set architecture riscv:rv64
target remote 127.0.0.1:1234

# Skip direct calls with synthetic frames; start tracing actual CPU exceptions.
hbreak lab3_illegal32
continue
delete 1
set $entries = 0
set $returns = 0
set $resumes = 0

hbreak exception_handler
commands
    silent
    set $frame = (struct trapframe *)tf
    if $frame->cause == 2 || $frame->cause == 3
        set $entries = $entries + 1
        set $old_epc = $frame->epc
        set $saved_sp = $frame->gpr.sp
        set $length = ((*(unsigned short *)$old_epc & 3) == 3) ? 4 : 2
        printf "[TRACE enter] cause=%lu EPC=0x%lx length=%d sepc=0x%lx stval=0x%lx\n", $frame->cause, $old_epc, $length, $sepc, $stval
        x/1xh $old_epc
    else
        printf "[TRACE synthetic fallback] cause=%lu EPC=0x%lx\n", $frame->cause, $frame->epc
    end
    continue
end

hbreak __trapret
commands
    silent
    set $returns = $returns + 1
    printf "[TRACE return] EPC=0x%lx delta=%lu saved_sp=0x%lx\n", $frame->epc, $frame->epc - $old_epc, $saved_sp
    if $frame->epc != $old_epc + $length
        printf "[TRACE ERROR] incorrect EPC advance\n"
        quit 1
    end
    continue
end

hbreak lab3_illegal32_resume
hbreak lab3_illegal16_resume
hbreak lab3_breakpoint16_resume
hbreak lab3_breakpoint32_resume
commands 4 5 6 7
    silent
    set $resumes = $resumes + 1
    printf "[TRACE resume] PC=0x%lx SP=0x%lx t0=0x%lx a0=0x%lx s0=0x%lx\n", $pc, $sp, $t0, $a0, $s0
    if $pc != $old_epc + $length || $sp != $saved_sp
        printf "[TRACE ERROR] incorrect resumed PC or SP\n"
        quit 1
    end
    continue
end

hbreak sbi_shutdown
commands
    silent
    printf "[TRACE summary] entries=%d returns=%d resumes=%d\n", $entries, $returns, $resumes
    if $entries != 4 || $returns != 4 || $resumes != 4
        printf "[TRACE ERROR] expected exactly four real exceptions\n"
        quit 1
    end
    printf "[TRACE] All EPC/PC/SP checks: PASS\n"
    detach
    quit
end
continue
