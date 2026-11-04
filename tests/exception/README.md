# 林子媛 2410936：异常处理手动调试

## 1. 提交代码与测试代码分开

- 提交实现：`../../code/kern/trap/trap.c` 中的 `exception_handler()`，保留 `lab3`，标注 `2410936`。
- 队友的 `RESTORE_ALL`、`__alltraps`、`interrupt_handler()` 尚未完成，未冒填学号或实现。
- `prepare.py` 把 `code/` 复制到新的测试目录，仅在副本中接入测试。
- 若入口未完成，副本使用 `reference-entry.S`，复用原框架的 `SAVE_ALL`。中断桩遇到中断直接报错，测试在启用时钟之前完成。
- `lab3_exception_test.c`、`probes.S` 只用于测试，不修改原评分脚本和评分条件。本次结果不能写成“队友代码已验证”或“make grade 满分”。

当前容器：`codex-lab2-lzy-debug-arm64-01a15ea8`，Ubuntu 22.04 / aarch64。
工具：RISC-V GCC 10.2.0、QEMU 6.2.0、GDB 12.1。QEMU 默认启动实际显示 OpenSBI v0.9。

容器中的 `/workspace/lab3` 是 Mac 文件的副本，不是共享挂载。不要在原 Lab2 共享目录 `/lab` 操作。
如果以后修改 Mac 上的实现，要先在 Mac 终端同步，再生成新的测试目录：

```bash
docker cp /Users/akobayashi/OSLab/Lab3/code/. codex-lab2-lzy-debug-arm64-01a15ea8:/workspace/lab3/code
docker cp /Users/akobayashi/OSLab/Lab3/tests/. codex-lab2-lzy-debug-arm64-01a15ea8:/workspace/lab3/tests
```

## 2. 手动生成副本、编译和运行

在 Mac 终端 A 进入 Ubuntu：

```bash
docker exec -it codex-lab2-lzy-debug-arm64-01a15ea8 bash
```

如果提示容器没启动，先在 Mac 执行 `docker start codex-lab2-lzy-debug-arm64-01a15ea8`。

以下命令都在 Ubuntu 中执行：

```bash
cd /workspace/lab3
python3 tests/exception/prepare.py /workspace/lab3-lzy-manual
cd /workspace/lab3-lzy-manual
make -j2
make qemu | tee run.log
grep -E 'LZY real|LZY synthetic|All exception' run.log
```

`prepare.py` 拒绝覆盖现有目录；如果手动目录已经存在，换用 `lab3-lzy-manual-2`，后面的路径也相应修改，不必删除旧证据。

**截图点 1：** 构建末尾的 `+ ld bin/kernel` 和汇总中的四条真实异常返回 `PASS`、四条构造帧 EPC `PASS`、两条兜底 `PASS`、最终 `All exception tests: PASS`。

## 3. 启动 GDB，跳过构造帧测试

终端 A 中执行：

```bash
make debug
```

此时暂停等待调试器，不是卡死。保持终端 A 开着。

新开 Mac 终端 B：

```bash
docker exec -it -w /workspace/lab3-lzy-manual codex-lab2-lzy-debug-arm64-01a15ea8 gdb-multiarch -q bin/kernel
```

随后在 `(gdb)` 提示符逐条输入：

```gdb
set pagination off
set logging file manual-gdb.log
set logging enabled on
target remote 127.0.0.1:1234
hbreak lab3_illegal32
continue
delete 1
hbreak exception_handler
continue
```

第一次断在 `lab3_illegal32` 是为了跳过前面的直接调用测试；第二次停在 `exception_handler` 才是 CPU 实际触发的第一种异常。

## 4. 每种真实异常重复三步

### A. 查看进入处理函数时的状态

```gdb
set $frame = tf
set $old_epc = $frame->epc
p/x $frame->cause
p/x $frame->epc
x/2xh $old_epc
p/d ((*(unsigned short *)$old_epc & 3) == 3 ? 4 : 2)
p/x $sepc
p/x $stval
p/x $frame->gpr.sp
```

**截图点 2：** 第一例的 `cause=2`、原 EPC、半字 `0x007b 0x0000` 和长度 `4`。
第三例应为 `cause=3`、半字 `0x9002` 和长度 `2`，也要保存截图。

### B. 查看 C 处理完成后的 Trap frame

```gdb
thbreak __trapret
continue
p/x $frame->epc
p/d $frame->epc - $old_epc
p/x $sepc
```

**截图点 3：** 第一例 EPC 增量 `4`，第三例增量 `2`。
此时还没执行 `RESTORE_ALL`，所以 `tf->epc` 已更新，而 `$sepc` 仍是旧地址；这不代表更新失败。

不要用 `finish` 代替此处的返回断点。预演时它曾越过当前返回，停在下一次异常；Trap frame 所在栈空间会复用，继续读取旧指针会误把下一次 EPC 当作本次结果。

### C. 查看 sret 后真正恢复的执行位置

```gdb
thbreak *$frame->epc
continue
p/x $pc
p/x $sp
p/x $frame->gpr.sp
info registers t0 a0 s0
```

**截图点 4：** `$pc` 等于新 EPC，`$sp` 等于保存的 `gpr.sp`，`t0=0x105`、`a0=0x10a`、`s0=0x108`。
汇编探针随后还会核对其他通用寄存器，恢复调用者状态并返回 C 测试函数。

前三例完成 C 步骤后输入 `continue`，就会停在下一例的 `exception_handler`，再重复 A/B/C。

| 顺序 | 探针 | 触发编码 | cause | EPC 增量 |
| --- | --- | --- | --- | --- |
| 1 | `lab3_illegal32` | `.4byte 0x0000007b` | 2 | 4 |
| 2 | `lab3_illegal16` | `.2byte 0x0000` | 2 | 2 |
| 3 | `lab3_breakpoint16` | `.2byte 0x9002`，`c.ebreak` | 3 | 2 |
| 4 | `lab3_breakpoint32` | `.4byte 0x00100073`，`ebreak` | 3 | 4 |

第四例完成 C 步骤后，不再重复，输入：

```gdb
delete breakpoints
detach
quit
```

如果询问是否删除全部断点，输入 `y`。终端 A 继续执行两个构造帧兜底测试，打印总结果并关机。

**截图点 5：** 终端 A 的 `cause 0x00000005`、`cause 0x00000014`、EPC 不变的 `PASS` 及最终总结果。这两项必须写明“构造 Trap frame 测试”，不是触发真实访存异常后恢复。

## 5. 保存日志和结论

在 Mac 终端复制本人手动生成的日志：

```bash
docker cp codex-lab2-lzy-debug-arm64-01a15ea8:/workspace/lab3-lzy-manual/run.log /Users/akobayashi/OSLab/Lab3/report/evidence/2410936/manual-run.log
docker cp codex-lab2-lzy-debug-arm64-01a15ea8:/workspace/lab3-lzy-manual/manual-gdb.log /Users/akobayashi/OSLab/Lab3/report/evidence/2410936/manual-gdb.log
```

截图保存在 `/Users/akobayashi/OSLab/Lab3/report/images/2410936/`。在个人报告 5.3 填入自己这次看到的 EPC、截图、预期与实际结果；地址可能随代码变化，不必与自动预演完全相同。

结论应区分：异常处理函数的分类、提示和 EPC 更新已通过个人测试；真实返回是在测试用入口/恢复依赖下验证；其他异常只验证兜底输出和不修改 Trap frame；全组 `make grade` 及队友最终入口/中断实现仍待合并后复测。

后续 Git 操作应进入 `/Users/akobayashi/OSLab/Lab3`，不要在外层 `OSLab` 仓库直接添加所有文件。
