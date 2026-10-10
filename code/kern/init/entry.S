#include <mmu.h>
#include <memlayout.h>

    .section .text,"ax",%progbits
    .globl kern_entry #使ld看到
kern_entry:
    la sp, bootstacktop
    tail kern_init #跳转到这里

.section .data
   # .align 2^12
    .align PGSHIFT
    .global bootstack #内核栈
bootstack:
    .space KSTACKSIZE #留栈内存
    .global bootstacktop
bootstacktop: 