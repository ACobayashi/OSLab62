# 2410936 异常处理：自动验证记录

## 2026-11-06 补交说明

本次按林子媛要求，将本目录的验证说明、原始日志、源码指纹与实现补丁补交为报告附件。以下正文及其他记录中“尚未提交”“只留本地”等文字保留的是各记录生成时的状态，不代表本次补交范围。

- 最终整合版结果见 [最终整合复测记录](./final-validation-20261106.md)；下文早期参考入口测试不能代替该次正式代码验证。
- 本人手动检查见 [手动验证进度](./manual-notes.md)，与工具自动执行的日志分别记录。
- `code-README-local.md` 是早期源码说明快照，其中路径相对于当时的代码目录；`source.sha256` 和 `exception-handler.patch` 也保留原验证阶段的内容，不作为最终整合版的完整指纹或补丁。
- 手动记录保留了全部本地截图的索引；本次不额外上传其中未被个人报告选用的图片。已提交的十张报告配图位于 `../../images/2410936/`。

## 原始自动验证记录

日期：2026-10-29。以下由 Codex 在 Docker Ubuntu 中执行，**不是林子媛本人手动操作的记录**。未生成或冒充终端截图；本人手动验证进度另见 [manual-notes.md](./manual-notes.md)，手动步骤见 `../../../tests/exception/README.md`。

## 分支与修改范围

- 新目录：`/Users/akobayashi/OSLab/Lab3`，本地分支 `lab3`，基线 `1bb05b7572611aff0e2c5ccbdb12de57ca31ee32`。
- Lab3 与 Lab2 从共同基础 `73ab335` 分开；Lab3 的 `3c7f635` 将 Lab2 提交 `a62570c` 的页初始化、Best Fit 和启动基础移入新框架，并非直接继承 Lab2 的完整提交历史。
- Lab3 增加 Trap、时钟和中断保护，未携带 Lab2 的 Buddy/SLUB 扩展，因此单独建立 Lab3，不覆盖原 Lab2 工作目录或未提交报告。
- 实现代码只修改 `code/kern/trap/trap.c` 的功能二区域：增加 `exception_handler()`，标注 `lab3 function2 2410936`；为队友的中断函数保留声明和待完成标记。
- `code/README.md` 更新个人完成状态；测试材料全部放在 `tests/exception/`，测试注入只作用于副本。原评分代码、框架结构和 `SAVE_ALL` 未修改。
- 本记录于 2026-10-29 初次生成时尚未提交或推送；队友的三个待完成位置没有替他们实现或署名。

2026-11-04 阶段提交范围：本人实现、隔离自测工具、自动验证记录及已有手动截图。提交前核对 `source.sha256` 的六项源码指纹均一致；本次没有重新运行或接管手动调试会话。本人手动 GDB 进度仍以 `manual-notes.md` 为准，第四例返回检查和个人报告尚待收尾。

## 实现约束

1. `cause=2` 输出 `Illegal instruction at <EPC>`；`cause=3` 输出 `Breakpoint at <EPC>`。
2. 对本次 RV64IMAFDC 目标，读取故障指令首个 16 位半字，低两位不为 `11` 时前进 2 字节，否则前进 4 字节。不是按异常类别硬编码长度，也不是所有异常统一加同一偏移。
3. 只更新 `tf->epc`，由统一返回路径恢复 `sepc`；不直接写 CSR，不修改寄存器快照和其他 Trap frame 字段。
4. 其他异常只调用原 `print_trapframe(tf)`；不读取其 EPC 指向的内存，不修改 EPC，不实现未要求的恢复。
5. 前提是本实验中引发非法指令/断点的内核代码可读取；不宣称实现任意用户地址安全取指或 48 位以上的通用指令解码。

## 实际环境

- 容器：`codex-lab2-lzy-debug-arm64-01a15ea8`，Ubuntu 22.04，aarch64。
- RISC-V GCC 10.2.0；binutils 2.35.1；QEMU 6.2.0；GDB multiarch 12.1。
- 实际 QEMU 启动画面为 OpenSBI v0.9，不能用已安装的 `opensbi` 软件包版本代替实际固件版本。
- ELF 属性：`rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0`，栈对齐 16 字节。
- 构建目录：`/workspace/lab3-lzy-test`；干净复测目录：`/workspace/lab3-lzy-clean`。
- 提交源目录 `/workspace/lab3/code` 与上述测试目录分离，原 Lab2 共享目录 `/lab` 未用于本次构建。

## 实际执行及结果

在容器 `/workspace/lab3` 执行：

```bash
python3 tests/exception/prepare.py /workspace/lab3-lzy-test
make -C /workspace/lab3-lzy-test -j2
timeout 20s make -C /workspace/lab3-lzy-test qemu
```

构建退出 0，QEMU 在超时前正常关机，退出 0，打印 `All exception tests: PASS`。随后用新目录 `lab3-lzy-clean` 重做上述步骤，构建与测试再次退出 0。

### 真实异常的 GDB 证据

| 场景 | 实际编码 | cause | 进入时 EPC | 处理后 EPC = 恢复 PC | 增量 |
| --- | --- | --- | --- | --- | --- |
| 32 位非法指令 | `0x0000007b` | 2 | `0xffffffffc0201e9a` | `0xffffffffc0201e9e` | 4 |
| 16 位非法指令 | `0x0000` | 2 | `0xffffffffc0202042` | `0xffffffffc0202044` | 2 |
| 压缩断点 | `0x9002` | 3 | `0xffffffffc02021ea` | `0xffffffffc02021ec` | 2 |
| 非压缩断点 | `0x00100073` | 3 | `0xffffffffc0202392` | `0xffffffffc0202396` | 4 |

四例在 GDB 中均观察到：保存的 SP 和返回后的 SP 都为 `0xffffffffc0205f20`，`t0=0x105`、`a0=0x10a`、`s0=0x108`。汇编探针还检查其余通用寄存器值，并保留调用者的被调用者保存寄存器。两条 32 位故障指令都刻意放在仅两字节对齐的地址，长度判断没有依赖四字节对齐读取。

`trace.gdb` 从第一条真实异常探针开始，跳过前面的构造帧测试；自动计数结果为 `entries=4 returns=4 resumes=4`，EPC/PC/SP 检查全部 PASS，没有反复执行同一故障指令。

实际观察到非法指令 32 位时 `stval=0x7b`，非法指令 16 位和两个断点时 `stval=0`。实现没有假定 `stval` 永远提供非零指令编码。

### 构造帧测试

- 四种编码分别直接调用 `exception_handler()`，比较整个 Trap frame，除预期 EPC 更新外其余字节不变，全部 PASS。
- `cause=5` 和 `cause=20`，EPC 均设为 0；兜底输出完整 Trap frame，函数返回，整个帧保持不变，均 PASS。
- 兜底两项是构造帧调用，不是触发真实访存异常后恢复；本题没有要求这类异常的恢复。

## 真实遇到的问题与修正

本次异常处理实现首次测试即通过，当前验证范围内未观察到函数实现本身的 bug，不编造代码修复迭代。

手动流程预演的第一次尝试使用 `finish`。它没有停在当前异常返回处，而是命中了下一次 `exception_handler`；共享的 Trap frame 栈空间已复用，于是读到的 EPC 差为 424，而不是当前指令的 4。这是调试方法产生的错误观察，不是已经证实的 EPC 实现错误。

改用 `thbreak __trapret`、`continue` 明确停在本次处理完成、恢复开始之前，再对新 EPC 设置临时硬件断点。复测确认增量 4、返回 PC 正确、SP 恢复；四次完整自动跟踪也全部通过。`trap-disassembly.log` 可见实际分发为尾跳转，且入口是汇编路径，因此不依赖自动栈展开来判断此处返回。

## 不能据此宣称的结果

提交目录 `make` 实际能链接，但这不等于 Trap 通路完整：原 `__alltraps` 的 C 调用仍为空，`--gc-sections` 将 `trap`、`exception_handler` 等未被入口引用的处理路径移除了。`submission-symbols.log` 中不存在这些符号。

提交版短跑到 `++ setup timer interrupts` 后无统计输出，由 3 秒 `timeout` 终止，退出 124。不能把它写成异常测试通过。本次个人测试使用了明确标示的参考入口/恢复依赖，未验证韩羽宸尚未提交的最终实现，未验证马禹翔的时钟处理，未执行或宣称全组 `make grade` 满分。

## 原始证据索引

- `build.log`、`qemu.log`：首次编译和运行。
- `clean-prepare.log`、`clean-build.log`、`clean-qemu.log`：新副本的干净复测。
- `gdb-trace.log`、`gdb-qemu.log`：四次真实异常的完整 GDB 跟踪和对应控制台。
- `manual-first-attempt.log`：使用 `finish` 时的真实误停记录。
- `manual-check.log`、`manual-check-qemu.log`：修正后手动命令的预演结果。
- `submission-build.log`、`submission-symbols.log`、`submission-qemu.log`：未补齐队友函数的提交版实际状态。
- `trap-disassembly.log`：测试版真实分发指令。
- `source.sha256`、`exception-handler.patch`：源码指纹和本人实现差异。

本人手动复现后的日志与截图应另行保存，不替换上述自动预演的来源说明。
