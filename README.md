# Lab1：最小可执行内核

## 实验内容

构建能够启动并输出信息的最小 RISC-V64 内核，分析交叉编译、链接脚本、ELF 文件和内核镜像的生成过程，使用 QEMU 验证内核启动及 SBI 输出。

通过 GDB 跟踪复位代码、OpenSBI 和内核入口，观察栈指针的设置、C 语言初始化及相关寄存器的变化。

## 目录结构

```text
.
|-- code/
|   |-- kern/             # 内核入口、初始化和控制台输出
|   |-- libs/             # SBI 接口、格式化输出等基础函数
|   |-- tools/            # 链接脚本和构建辅助规则
|   `-- Makefile          # 编译、运行与调试规则
|-- report/
|   |-- images/           # 运行与调试截图
|   |-- prompt.md         # AI 提示词记录
|   |-- report.md         # 实验报告
|   `-- report.pdf        # 实验报告 PDF
`-- README.md
```
