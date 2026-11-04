    .text

    .macro CHECK_REGISTER reg, value, failure
    li a0, \value
    bne \reg, a0, \failure
    .endm

    .macro EXCEPTION_PROBE name, bytes, opcode
    .p2align 2
    .globl \name
\name:
    addi sp, sp, -144
    sd s0, 0(sp)
    sd s1, 8(sp)
    sd s2, 16(sp)
    sd s3, 24(sp)
    sd s4, 32(sp)
    sd s5, 40(sp)
    sd s6, 48(sp)
    sd s7, 56(sp)
    sd s8, 64(sp)
    sd s9, 72(sp)
    sd s10, 80(sp)
    sd s11, 88(sp)
    sd ra, 96(sp)
    sd gp, 112(sp)
    sd tp, 120(sp)
    li t0, 0x105
    li t1, 0x106
    li t2, 0x107
    li s0, 0x108
    li s1, 0x109
    li a0, 0x10a
    li a1, 0x10b
    li a2, 0x10c
    li a3, 0x10d
    li a4, 0x10e
    li a5, 0x10f
    li a6, 0x110
    li a7, 0x111
    li s2, 0x112
    li s3, 0x113
    li s4, 0x114
    li s5, 0x115
    li s6, 0x116
    li s7, 0x117
    li s8, 0x118
    li s9, 0x119
    li s10, 0x11a
    li s11, 0x11b
    li t3, 0x11c
    li t4, 0x11d
    li t5, 0x11e
    li t6, 0x11f
    /* Exercise 32-bit opcodes at halfword-aligned, not word-aligned addresses. */
    .p2align 2
    .2byte 0x0001
    .globl \name\()_site
\name\()_site:
    .if \bytes == 2
    .2byte \opcode
    .else
    .4byte \opcode
    .endif
    .globl \name\()_resume
\name\()_resume:
    sd a0, 104(sp)
    CHECK_REGISTER t0, 0x105, .Lfail\@
    CHECK_REGISTER t1, 0x106, .Lfail\@
    CHECK_REGISTER t2, 0x107, .Lfail\@
    CHECK_REGISTER s0, 0x108, .Lfail\@
    CHECK_REGISTER s1, 0x109, .Lfail\@
    CHECK_REGISTER a1, 0x10b, .Lfail\@
    CHECK_REGISTER a2, 0x10c, .Lfail\@
    CHECK_REGISTER a3, 0x10d, .Lfail\@
    CHECK_REGISTER a4, 0x10e, .Lfail\@
    CHECK_REGISTER a5, 0x10f, .Lfail\@
    CHECK_REGISTER a6, 0x110, .Lfail\@
    CHECK_REGISTER a7, 0x111, .Lfail\@
    CHECK_REGISTER s2, 0x112, .Lfail\@
    CHECK_REGISTER s3, 0x113, .Lfail\@
    CHECK_REGISTER s4, 0x114, .Lfail\@
    CHECK_REGISTER s5, 0x115, .Lfail\@
    CHECK_REGISTER s6, 0x116, .Lfail\@
    CHECK_REGISTER s7, 0x117, .Lfail\@
    CHECK_REGISTER s8, 0x118, .Lfail\@
    CHECK_REGISTER s9, 0x119, .Lfail\@
    CHECK_REGISTER s10, 0x11a, .Lfail\@
    CHECK_REGISTER s11, 0x11b, .Lfail\@
    CHECK_REGISTER t3, 0x11c, .Lfail\@
    CHECK_REGISTER t4, 0x11d, .Lfail\@
    CHECK_REGISTER t5, 0x11e, .Lfail\@
    CHECK_REGISTER t6, 0x11f, .Lfail\@
    ld t0, 104(sp)
    CHECK_REGISTER t0, 0x10a, .Lfail\@
    ld a0, 96(sp)
    bne ra, a0, .Lfail\@
    ld a0, 112(sp)
    bne gp, a0, .Lfail\@
    ld a0, 120(sp)
    bne tp, a0, .Lfail\@
    li a0, 0
    j .Ldone\@
.Lfail\@:
    li a0, 1
.Ldone\@:
    ld s0, 0(sp)
    ld s1, 8(sp)
    ld s2, 16(sp)
    ld s3, 24(sp)
    ld s4, 32(sp)
    ld s5, 40(sp)
    ld s6, 48(sp)
    ld s7, 56(sp)
    ld s8, 64(sp)
    ld s9, 72(sp)
    ld s10, 80(sp)
    ld s11, 88(sp)
    ld ra, 96(sp)
    ld gp, 112(sp)
    ld tp, 120(sp)
    addi sp, sp, 144
    ret
    .endm

    EXCEPTION_PROBE lab3_illegal32, 4, 0x0000007b
    EXCEPTION_PROBE lab3_illegal16, 2, 0x0000
    EXCEPTION_PROBE lab3_breakpoint16, 2, 0x9002
    EXCEPTION_PROBE lab3_breakpoint32, 4, 0x00100073
