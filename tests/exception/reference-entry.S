/* Test fixture only; reuse the framework's exact SAVE_ALL and frame layout. */
#define __alltraps lab3_unused_skeleton_entry
#define __trapret lab3_unused_skeleton_return
#include "trapentry-skeleton.inc"
#undef __alltraps
#undef __trapret

    .purgem RESTORE_ALL
    .macro RESTORE_ALL
    LOAD t0, 32*REGBYTES(sp)
    LOAD t1, 33*REGBYTES(sp)
    csrw sstatus, t0
    csrw sepc, t1
    LOAD x1, 1*REGBYTES(sp)
    LOAD x3, 3*REGBYTES(sp)
    LOAD x4, 4*REGBYTES(sp)
    LOAD x5, 5*REGBYTES(sp)
    LOAD x6, 6*REGBYTES(sp)
    LOAD x7, 7*REGBYTES(sp)
    LOAD x8, 8*REGBYTES(sp)
    LOAD x9, 9*REGBYTES(sp)
    LOAD x10, 10*REGBYTES(sp)
    LOAD x11, 11*REGBYTES(sp)
    LOAD x12, 12*REGBYTES(sp)
    LOAD x13, 13*REGBYTES(sp)
    LOAD x14, 14*REGBYTES(sp)
    LOAD x15, 15*REGBYTES(sp)
    LOAD x16, 16*REGBYTES(sp)
    LOAD x17, 17*REGBYTES(sp)
    LOAD x18, 18*REGBYTES(sp)
    LOAD x19, 19*REGBYTES(sp)
    LOAD x20, 20*REGBYTES(sp)
    LOAD x21, 21*REGBYTES(sp)
    LOAD x22, 22*REGBYTES(sp)
    LOAD x23, 23*REGBYTES(sp)
    LOAD x24, 24*REGBYTES(sp)
    LOAD x25, 25*REGBYTES(sp)
    LOAD x26, 26*REGBYTES(sp)
    LOAD x27, 27*REGBYTES(sp)
    LOAD x28, 28*REGBYTES(sp)
    LOAD x29, 29*REGBYTES(sp)
    LOAD x30, 30*REGBYTES(sp)
    LOAD x31, 31*REGBYTES(sp)
    LOAD x2, 2*REGBYTES(sp)
    .endm

    .globl __alltraps
    .p2align 2
__alltraps:
    SAVE_ALL
    mv a0, sp
    call trap
    .globl __trapret
__trapret:
    RESTORE_ALL
    sret
