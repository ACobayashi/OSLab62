#!/bin/sh
# LAB4 function3 2410933
# 加载内核与 RV64 架构，兼容 QEMU 4.1.1 的寄存器描述。
exec gdb-multiarch -ex 'set architecture riscv:rv64' -ex 'file bin/kernel' "$@"
