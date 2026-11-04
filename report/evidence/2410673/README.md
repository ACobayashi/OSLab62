# 韩羽宸功能一：证据与复现

对应 [个人报告](../../2410673_韩羽宸.md) 的 4.1、5.1、5.5。原始实现与验证日期为 2026-11-04；本目录在整理报告时复制已有证据并补充便于复现的准备脚本。脚本整理不计为新的功能代码迭代。

## 已有证据

| 材料 | 内容 |
| --- | --- |
| [trapentry.S.snapshot](trapentry.S.snapshot) | 本次真实被测的正式汇编快照，不是参考入口 |
| [异常联调.log](异常联调.log) | 四类真实异常返回，以及合成帧的 EPC 与兜底检查 |
| [上下文状态编译.log](上下文状态编译.log) | 上下文副本的实际构建、链接与镜像生成 |
| [上下文状态复核.log](上下文状态复核.log) | 两种 SIE 状态、总计 258 次真实进入/返回的完整输出 |
| [本地提交记录.json](本地提交记录.json) | 本人代码提交号、提交内容和未推送状态 |
| [fixtures](fixtures/) | 实际使用的上下文断言、ABI 安全探针、修改 a0 的探针、异常驱动及中断失败桩 |
| [prepare_hyc.py](prepare_hyc.py) | 从正式 code 创建新的隔离验证副本；不修改正式代码，不替换真实汇编 |
| [报告整理复现记录.json](报告整理复现记录.json) | 用随附脚本重新准备两种副本，实际编译运行均通过，退出码均为 0 |
| [报告整理复现-context.log](报告整理复现-context.log)、[报告整理复现-exception.log](报告整理复现-exception.log) | 整理报告时对上述复现方法再次实际执行所得串口日志 |

正式汇编及两个原始验证副本的 SHA-256：

```text
639770e6099b28225103e63e528adc41ce735889584a6b174098eb2d8db88405
```

## 复现命令

在 WSL Ubuntu-22.04 中执行。原验证副本仍保留在本地附加材料中；若需要从当前正式代码重新准备，使用下列脚本。输出目录必须是新目录，脚本不会删除或覆盖已有目录。若以后正式汇编变化，脚本会拒绝与快照不一致的版本，需重新审查后记录新的验证对象。

```bash
export PATH=/home/smile/toolchains/riscv-elf-toolchains/bin:$PATH
LAB3='/mnt/e/Grade3_1/操作系统/os_lab/lab3'
python3 "$LAB3/report/evidence/2410673/prepare_hyc.py" \
  "$LAB3/附加材料/韩羽宸报告整理/复现-context" --mode context
cd "$LAB3/附加材料/韩羽宸报告整理/复现-context"
make -j2
/usr/local/bin/qemu-system-riscv64 \
  -machine virt -nographic -bios default \
  -kernel bin/ucore.img \
  -device loader,file=bin/ucore.img,addr=0x80200000
printf 'QEMU_EXIT_STATUS=%s\n' "$?"
```

预期最后出现两种 SIE 的 PASS、`258 entry/return cycles: PASS`，QEMU 退出码应为 0。前两条各写 128 次普通异常；每种状态还分别有一次修改 a0 的真实断点，总数为 `2×(128+1)=258`。

如需复现四类异常联调，将准备命令的目标目录改成一个新的 `复现-exception`，模式改为 `--mode exception`，然后在该目录执行同样的编译运行命令。该模式不加入 HYC 帧观察调用；它使用已有 LZY 异常驱动与本人的真实汇编。日志中的 `reference entry/return may be used` 是原驱动的通用说明；本脚本不会复制参考入口，且明确输出 `REFERENCE_ENTRY_USED=no`。

## 验证配置和边界

- 测试入口位于第二次 `idt_init()` 后、`clock_init()` 前；验证成功后调用 SBI 关机，不进入时钟循环。
- 若正式代码还没有中断函数定义，副本才加入 `interrupt-stub.c`。它在出现意外中断时立即 panic，不实现计数或时钟续设。
- 上下文模式将 `sie=0`，在 SIE=0/1 下触发同步异常，用于检查状态保存恢复；不代表实际时钟中断通过。
- 两种模式都保留正式 `trapentry.S` 和队友异常函数。仅上下文模式在副本的 `trap()` 前后插入观察调用。
- 没有修改 `tools/grade.sh` 或评分条件；这些测试不是 `make grade`。
- 两张本人现场原图均已归档并插入个人报告：`report/images/2410673/01_真实异常与断点返回.png`、`report/images/2410673/02_Trap帧与入口参数.png`。第二张记录 C 入口帧与真实返回后的 PC/SP，未逐条单步完整恢复宏。

原始 QEMU 退出状态 0 来自单独复核命令的返回值，原始串口日志本身不包含 shell 的退出码行。后续现场复现时使用上面的 `printf` 同屏展示，不能把 shell 状态伪装成内核输出。
