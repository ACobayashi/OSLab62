# LAB4 function3 2410933
    .text
    .globl switch_to
    .type switch_to, @function

switch_to:
    # 保存旧线程现场；a0 指向 from。
    sd ra,   0(a0)
    sd sp,   8(a0)
    sd s0,  16(a0)
    sd s1,  24(a0)
    sd s2,  32(a0)
    sd s3,  40(a0)
    sd s4,  48(a0)
    sd s5,  56(a0)
    sd s6,  64(a0)
    sd s7,  72(a0)
    sd s8,  80(a0)
    sd s9,  88(a0)
    sd s10, 96(a0)
    sd s11,104(a0)

    # 恢复目标线程现场；a1 指向 to，后续不再访问旧栈。
    ld ra,   0(a1)
    ld sp,   8(a1)
    ld s0,  16(a1)
    ld s1,  24(a1)
    ld s2,  32(a1)
    ld s3,  40(a1)
    ld s4,  48(a1)
    ld s5,  56(a1)
    ld s6,  64(a1)
    ld s7,  72(a1)
    ld s8,  80(a1)
    ld s9,  88(a1)
    ld s10, 96(a1)
    ld s11,104(a1)

    # 跳转到目标 ra：首次运行进入 forkret，否则继续先前执行。
    ret
    .size switch_to, .-switch_to
