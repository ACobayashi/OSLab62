# Lab3 起始代码：中断与异常处理

本目录已补齐 Lab2 的内存管理基础，接下来完成 Lab3 的 Trap 功能。

## Lab2 来源与已补回内容

- 仓库：[ACobayashi/OSLab62，lab2 分支](https://github.com/ACobayashi/OSLab62/tree/lab2)。
- 来源提交：`a62570c22866100da9c6a946df72c95a8556d3cf`（整理最终提交）。
- `kern/mm/pmm.c`：补回 `page_init()`，默认使用已完成的 Best Fit；保留 Lab3 的中断保护。
- `kern/mm/best_fit_pmm.c`：补回完整分配器，原有两个检查函数保持原样。
- `kern/mm/memlayout.h`：补齐页初始化所需的内存布局常量。
- `kern/init/entry.S`：补回低地址临时恒等映射、跳入高地址和移除临时映射的启动流程。
- `Makefile`、`tools/grade.sh`：补齐 QEMU 的 `-kernel` 参数，提供 OpenSBI 下一阶段入口信息。
- `kern/trap/trapentry.S`：将旧 CSR 名称 `sbadaddr` 改为当前工具链支持的 `stval`，仍保存到 `trapframe.badvaddr`。

## 本次实验需要完成

按手册设计 spec、提示词，并保留实际实现与验证记录：

| 文件 | 待完成内容 |
| --- | --- |
| `kern/trap/trapentry.S` | `RESTORE_ALL`：与已有 `SAVE_ALL` 和 Trap frame 布局对应 |
| `kern/trap/trapentry.S` | `__alltraps`：传递 Trap frame，调用 C 函数 `trap()` |
| `kern/trap/trap.c` | `interrupt_handler()`：中断分类、时钟续设、计数、输出和规定条件下关机 |
| `kern/trap/trap.c` | `exception_handler()`：2410936 已完成，个人异常测试及 GDB 核验通过 |

保留 `lab3` 注释；`exception_handler()` 标记为实际实现者 `2410936`。功能二原共享标记已拆分，`interrupt_handler()` 的 `YOUR CODE` 和功能一的两个 `YOUR CODE` 留给对应负责人，尚未完成。

注意：当前直接 `make` 能编译链接，但不能据此判断 Lab3 已完成。尚未补齐的 `__alltraps` 没有调用 `trap()`，链接器的 `--gc-sections` 会移除未使用的处理路径。需要合并另两位成员的实现后再验证完整运行和 `make grade`。

2026-10-29 的个人验证使用隔离副本，其中入口/恢复由测试夹具提供，中断桩遇到中断立即报错。四种真实异常、构造 Trap frame 的 EPC/兜底测试以及 GDB 的 EPC/PC/SP 检查均通过；这不是小组最终评分。手动复现步骤见 [异常调试说明](../tests/exception/README.md)，原始日志见 [验证记录](../report/evidence/2410936/README.md)。

## 编译与验证

在 WSL 中进入本目录，使用已有工具链：

```bash
export PATH="/home/smile/toolchains/riscv-elf-toolchains/bin:$PATH"
make
```

完成 Lab3 功能后，再执行 `make qemu` 和 `make grade`。调试入口为 `make debug`、`make gdb`。

2026-10-28 已在独立验证副本中确认：编译链接成功、QEMU 启动成功、DTB 检测到 128 MiB 内存、Best Fit 的原有自检通过。该副本使用仅供验证的处理函数占位，并在 `pmm_init()` 后关机，以隔离尚未完成的 Lab3 功能；此结果只验证启动与 Lab2 基础。

调整前备份、改动 diff、验证副本和日志位于 `../附加材料/Lab3起始准备/`。
