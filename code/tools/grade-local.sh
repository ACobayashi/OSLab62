#!/bin/sh
# LAB4 function3 2410933
set -eu
cd "$(dirname "$0")/.."
# 使用原评分规则；仅兼容 GCC 15 的旧测试函数类型和本机 GDB。
exec make grade GDB="$PWD/tools/gdb-lab4.sh" DEFS=-Wno-error=incompatible-pointer-types
