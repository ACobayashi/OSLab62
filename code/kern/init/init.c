#include <stdio.h>
#include <string.h>
#include <sbi.h>
int kern_init(void) __attribute__((noreturn));

int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata); //清除.bss段

    const char *message = "(THU.CST) os is loading ...\n";
    cprintf("%s\n\n", message); //格式化输出函数
   while (1)
        ;
}
