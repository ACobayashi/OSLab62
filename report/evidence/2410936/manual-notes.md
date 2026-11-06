# 林子媛 2410936：本人手动验证进度

日期：2026-10-29 起，2026-10-30 继续补充，2026-11-06 根据最后一张截图更新。依据本人在终端执行命令后提供的实际截图记录；与同目录 Codex 自动预演日志分开。四种真实异常的 GDB 手动检查均已完成；早期截图使用参考入口，不能冒充最终整合版截图。

## 已完成：准备、编译与运行

- 从 `/workspace/lab3/code` 生成独立目录 `/workspace/lab3-lzy-manual`，使用明确标示的测试入口/恢复依赖和中断桩。
- `make -j2` 完成编译、链接和生成镜像，输出保存为容器内 `build.log`。
- `make qemu` 运行结束，输出保存为容器内 `run.log`；四项构造帧 EPC 检查、四项真实异常返回、两项兜底检查及总结果均打印 PASS。
- 上述 PASS 由 AI 辅助编写的自测程序断言后输出，不是仓库 `make grade` 的评分。`[LZY]` 是测试日志的人为标签，并非 Ubuntu 或 Makefile 自动生成。

## 已完成：32 位非法指令的 GDB 检查

1. 连接 QEMU，在 `lab3_illegal32` 暂停，跳过构造帧直接调用，再在 `exception_handler` 暂停。
2. 进入处理函数时：`cause=0x2`，`tf->epc=sepc=0xffffffffc0201e9a`；故障位置的两个半字为 `0x007b 0x0000`，组成编码 `0x0000007b`；`stval=0x7b`；保存的 SP 为 `0xffffffffc0205f20`。
3. 用 `thbreak __trapret` 检查处理后状态：`tf->epc=0xffffffffc0201e9e`，增量为 4；恢复开始前，`sepc` 仍为旧地址 `0xffffffffc0201e9a`。
4. 对新 EPC 设置临时硬件断点，执行到异常指令后第一条指令：`pc=sepc=0xffffffffc0201e9e`，`sp=$saved_sp=0xffffffffc0205f20`；`t0=0x105`、`a0=0x10a`、`s0=0x108`，与探针预设值一致。
5. 从返回点继续后，QEMU 控制台实际打印 `illegal32: continued, GPR/stack return: PASS`，随后开始 `illegal16`。这说明第一例的汇编探针完成比对并返回 C 测试函数；第二例此时只看到测试开始提示，实际异常状态及 EPC 更新仍待手动检查。

本例已确认 EPC 增量、实际返回位置、SP 以及上述三个寄存器的恢复。该阶段结论基于测试入口/恢复依赖，本身不代表队友最终汇编已经验证；最终代码另于 2026-11-06 自动复测。

## 已完成：16 位非法指令的 GDB 检查

1. 已在第二次真实异常的处理函数入口重新记录 Trap frame：`cause=0x2`，`tf->epc=sepc=0xffffffffc0202042`，故障半字为 `0x0000`，`stval=0`。
2. 在 `__trapret` 暂停，实际观察到 `tf->epc=0xffffffffc0202044`，增量为 2，且 QEMU 控制台输出本例的 `Illegal instruction at` 提示。
3. 输入 `p/x $ sepc` 时，美元符号和寄存器名之间误加空格，GDB 报 `A syntax error in expression, near 'sepc'.`。这是调试命令的输入错误，不是内核代码失败；该条读取没有取得结果，也没有继续执行内核。随后改为 `p/x $sepc`，实际读得 `0xffffffffc0202042`，确认恢复汇编开始前 CSR 仍为旧值。
4. 对新 EPC 设置临时硬件断点后，实际停在 `lab3_illegal16_resume`：`pc=sepc=0xffffffffc0202044`，`sp=$saved_sp=0xffffffffc0205f20`，`t0=0x105`、`a0=0x10a`、`s0=0x108`。本例的 EPC 增量、实际返回位置、SP 和上述寄存器已核对，均符合预期。
5. 随后继续，QEMU 控制台实际打印 `illegal16: continued, GPR/stack return: PASS`，确认第二例探针完成并返回 C 测试函数。

## 已完成：16 位断点的 GDB 检查

1. 在第三次真实异常的处理函数入口，实际读取 `cause=0x3`，`tf->epc=sepc=0xffffffffc02021ea`，故障半字为 `0x9002`（`c.ebreak`），`stval=0`。
2. 在 `__trapret` 实际观察到 `tf->epc=0xffffffffc02021ec`，增量为 2，`sepc` 尚为原地址 `0xffffffffc02021ea`；QEMU 实际输出 `Breakpoint at 0xffffffffc02021ea`，确认进入断点提示分支。
3. 对新 EPC 设置临时硬件断点后，实际停在 `lab3_breakpoint16_resume`：`pc=sepc=0xffffffffc02021ec`，`sp=$saved_sp=0xffffffffc0205f20`，`t0=0x105`、`a0=0x10a`、`s0=0x108`。本例的断点分类提示、EPC 增量、实际返回位置、SP 和上述寄存器均符合预期。
4. 随后继续，QEMU 实际输出 `breakpoint16: continued, GPR/stack return: PASS`，确认第三例探针完成并返回 C 测试函数。

## 已完成：32 位断点的 GDB 检查

1. 在第四次真实异常的处理函数入口，实际读取 `cause=0x3`，`tf->epc=sepc=0xffffffffc0202392`，两个半字为 `0x0073 0x0010`，组成 `0x00100073`（32 位 `ebreak`），`stval=0`。
2. 2026-11-06 提供的最后截图显示，在 `__trapret` 处帧 EPC 为 `0xffffffffc0202396`，差值为 4，恢复前 `sepc` 仍为 `0xffffffffc0202392`。
3. 继续后实际停在 `lab3_breakpoint32_resume`：`pc=sepc=0xffffffffc0202396`，`sp=$saved_sp=0xffffffffc0205f20`，`t0=0x105`、`a0=0x10a`、`s0=0x108`。第四例三个检查点均已核对。
4. 该截图没有展示随后退出 GDB 和关机的操作，不补写为已看到；此前不挂 GDB 的手动完整运行已经显示全部自测 PASS。

## 截图索引

- `../../images/2410936/01_build.png`：手动创建测试副本并编译成功。
- `../../images/2410936/02_exception_tests.png`：手动运行后的自测汇总。
- `../../images/2410936/03_gdb_handler_entry.png`：真实探针进入异常处理函数。
- `../../images/2410936/03_qemu_paused.png`：对应 QEMU 控制台状态。
- `../../images/2410936/04_illegal32_before.png`：异常原因、原 EPC、指令编码及保存的 SP。
- `../../images/2410936/05_illegal32_epc_updated.png`：处理后 EPC 增量为 4。
- `../../images/2410936/05_illegal32_console.png`：测试提示和处理函数输出的区别。
- `../../images/2410936/06_illegal32_resumed.png`：实际返回 PC、sepc、SP 和寄存器检查。
- `../../images/2410936/07_illegal32_completed_illegal16_started.png`：第一例探针完成，第二例开始。
- `../../images/2410936/08_illegal16_before.png`：第二例的实际异常原因、原 EPC、指令半字和 stval。
- `../../images/2410936/09_illegal16_epc_updated.png`：第二例 EPC 增量为 2，以及一次 GDB 命令空格输入错误。
- `../../images/2410936/09_illegal16_console.png`：第二例的异常处理提示。
- `../../images/2410936/10_illegal16_sepc_before_restore.png`：修正读取命令后，确认处理后的帧 EPC 与尚未恢复的 sepc 不同。
- `../../images/2410936/11_illegal16_resumed.png`：16 位非法指令返回后的 PC、sepc、SP 和三个寄存器核对。
- `../../images/2410936/12_breakpoint16_before.png`：16 位断点的实际异常原因、原 EPC、指令半字和 stval。
- `../../images/2410936/13_breakpoint16_epc_updated.png`：16 位断点 EPC 增量为 2，恢复前 sepc 保持旧值。
- `../../images/2410936/13_breakpoint16_console.png`：断点提示及前一例非法指令探针完成的输出。
- `../../images/2410936/14_breakpoint16_resumed.png`：16 位断点返回后的 PC、sepc、SP 和三个寄存器核对。
- `../../images/2410936/15_breakpoint32_before.png`：32 位断点的实际异常原因、原 EPC、指令半字和 stval。
- `../../images/2410936/15_breakpoint32_started.png`：第三例探针完成并开始第四例的控制台输出。
- `../../images/2410936/16_breakpoint32_resumed.png`：最后一例 EPC 增加 4，实际返回后的 PC、sepc、SP 和三个寄存器均符合预期。

## 最终补充

- 2026-11-06，Codex 用全新目录复测最终整合版 `dbe2dde`：仓库评分 100/100、普通内核十次输出后正常关机、正式汇编下个人异常回归全部通过。这些是自动执行记录，不冒充本人手动操作。
- 个人报告已经整理 SPEC、测试设计、原始输入与复现提示词的来源，以及十张筛选后的关键截图。旧截图的绝对地址和参考入口依赖没有改写为最终版结果。
- 新增最终测试日志只留本地；本次未提交或推送报告、截图或日志。
