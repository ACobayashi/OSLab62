# Lab2：物理内存和页表

## 实验内容

在 Lab1 最小可执行内核的基础上，完善 uCore RISC-V64 内核的内存管理。实验包括物理内存探测与页初始化、Best Fit 页分配器的实现与接入，以及 Buddy System 的独立实现和验证。

在页级分配的基础上，实现 SLUB 对象分配器，并将启动过程切换到 Sv39 分页模式。通过内核测试、评分脚本和 QEMU/GDB 调试，验证分配器行为、页表映射和地址翻译过程，并观察 Bare 与 Sv39 模式下的 TLB 行为。

## 目录结构

```text
.
|-- code/
|   |-- kern/
|   |   |-- driver/       # 控制台与设备树信息处理
|   |   |-- init/         # 内核入口和启动初始化
|   |   `-- mm/           # 物理页管理、Best Fit、Buddy、SLUB 与分页
|   |-- libs/             # SBI、格式化输出及基础数据结构
|   |-- tools/            # 链接脚本、评分脚本和实验测试代码
|   `-- Makefile          # 编译、运行与测试规则
|-- report/
|   |-- images/           # 实验运行和调试截图
|   |-- prompt.md         # AI 提示词记录
|   `-- report.md         # 实验报告
`-- README.md
```

## 主要验证内容

- 物理内存范围探测、页初始化和 Best Fit 分配器测试
- Buddy System 的页分配与回收验证
- SLUB 不同对象大小、混合分配、回收及重复释放检查
- Sv39 启动分页过程，以及 QEMU/GDB 地址翻译和 TLB 调试
