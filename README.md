# 操作系统实验

本仓库用于 uCore on RISC-V64 操作系统实验。各实验分支分别保存对应的内核代码、实验报告、运行截图和 AI 提示词记录。

## 实验总览

| 实验 | 分支 | 内容 | 状态 |
|------|------|------|------|
| Lab1：最小可执行内核 | [lab1](https://github.com/ACobayashi/OSLab62/tree/lab1) | RISC-V64 内核启动、SBI 输出与 GDB 调试 | 已完成 |
| Lab2：物理内存和页表 | [lab2](https://github.com/ACobayashi/OSLab62/tree/lab2) | 物理页分配、Best Fit、Buddy、SLUB 与 Sv39 分页 | 已完成 |
| Lab3 | [lab3](https://github.com/ACobayashi/OSLab62/tree/lab3) | 操作系统后续实验内容 | 尚未开始 |

## 实验分支目录

每个实验分支按以下结构组织代码和实验资料：

```text
.
|-- code/
|   |-- kern/             # 内核代码
|   |-- libs/             # 基础库
|   `-- tools/            # 构建、运行和测试工具
|-- report/
|   |-- images/           # 实验运行与调试截图
|   |-- prompt.md         # AI 提示词记录
|   `-- report.md         # 实验报告
`-- README.md
```
