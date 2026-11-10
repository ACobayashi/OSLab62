# Lab4 起始代码：进程管理

已在本次框架内补回 Lab2 物理页初始化、Best Fit、启动分页过渡，以及 Lab3 Trap 入口/恢复、中断与异常处理。Lab1 的控制台与启动调试基础沿用当前框架已有内容。

## 本次待完成

- `kern/mm/pmm.c`：`page_insert`、`page_remove`。
- `kern/process/proc.c`：`alloc_proc`、`proc_init`、`kernel_thread`、`do_fork`、`copy_thread`、`copy_mm`、`cpu_idle`、`proc_run`。
- `kern/schedule/sched.c`：`schedule`。
- `kern/process/switch.S`：`switch_to`。

LAB4 / YOUR CODE 标记保持原样，尚未填写本次函数。个人任务和报告见 `../附加材料/lab4三人分工.md`、`../report/汇总.md`。

## 合并约定

- `default_pmm.c` 在原始 Lab4 中实际保留的是 Best Fit 自检。本次补入配套 Best Fit 实现，管理器名为 `best_fit_pmm_manager`，沿用原文件名和原有检查函数。
- 保留 Lab4 的 `Page` 扩展字段、`boot_pgdir_va/pa`、页表操作/VMA、SLOB `kmalloc/kfree`、进程和调度框架，以及 `forkrets`。Lab2 Buddy/SLUB 不替换本次框架的分配接口。
- 沿用 Lab3 时钟续设、计数输出和异常恢复；Lab3 的十次输出后关机属于上次验收条件，本次持续运行时钟，保留 Lab4 原有 DEBUG_GRADE 行为。
- QEMU 继承已验证的 `-kernel` 启动参数，评分条件和分值保持原样。

## 验证边界

正式起始代码尚缺上述 Lab4 函数，完整构建/链接、线程运行和评分需实现后执行。初始化时的隔离基础验证只证明继承的启动、物理页、Trap 和框架 VMA 基础；具体结果与日志见 `../附加材料/Lab4起始准备/补回说明.md`。
