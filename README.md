# Operating System Labs

本仓库用于 uCore on RISC-V64 操作系统实验。

## 实验分支

| 实验 | 分支 | 状态 |
|------|------|------|
| Lab 1 | [`lab1`](https://github.com/ACobayashi/OSLab62/tree/lab1) | 环境与模板已准备 |

每次实验使用独立的 `labx` 分支提交，例如 `lab1`、`lab2`。

## 分支目录规范

```text
.
|-- code/                 # 实验一源代码
|-- report/
|   |-- images/           # 测试结果截图
|   |-- prompt.md         # 本实验使用的全部提示词
|   `-- report.md         # 实验报告模板
|-- .gitignore
`-- README.md
```

每个实验分支只保留一份源代码和一份实验报告模板：

- `code/`：对应实验的源代码。
- `report/report.md`：实验报告。
- `report/prompt.md`：本实验使用的全部提示词。
- `report/images/`：实验报告中引用的测试截图。
