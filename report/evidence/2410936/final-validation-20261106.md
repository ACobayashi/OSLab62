# 2410936：最终整合复测记录

执行日期：2026-11-06。执行者：Codex 工具自动运行，不是林子媛本人手动截图。

- 验证版本：`dbe2dde2e1c13ef21f4587bd7bf1bd85a7b1e3cc`。
- 容器：`codex-lab2-lzy-debug-arm64-01a15ea8`，Ubuntu 22.04.2 LTS / aarch64。
- 新目录：`/workspace/lab3-lzy-final-20261106-dbe2dde`，原 `/lab` 和手动 GDB 目录未覆盖。
- `code`、`tests` 从当前 Mac 仓库复制；没有修改功能源码、评分脚本或评分条件。

## 命令和结果

以下命令在新目录中依次执行，全部退出 0：

```bash
cd /workspace/lab3-lzy-final-20261106-dbe2dde/code
make grade GRADE_TMP=/workspace/lab3-lzy-final-20261106-dbe2dde/grade-tmp
make -j2
timeout 20s make qemu
cd /workspace/lab3-lzy-final-20261106-dbe2dde
python3 tests/exception/prepare.py /workspace/lab3-lzy-final-20261106-dbe2dde/exception
cd exception
make -j2
timeout 20s make qemu
```

- 评分分别为 35/35、25/25、40/40，总分 100/100。评分脚本按其原有设计使用 timeout 收集输出，不能把总命令退出 0 解释成三个评分 QEMU 进程均自行关机。
- 普通内核运行实际输出十次 `100 ticks` 后自行关机；外层 timeout 命令退出 0，不是 124。
- 异常回归：四项构造帧、四项真实异常、两项兜底检查以及总结果全部 PASS，QEMU 命令退出 0。
- prepare 输出没有参考入口/中断桩的 TEST ONLY 提示；`exception/kern/test` 只有 C 驱动、头文件与 `probes.S`。驱动固定的 may-be-used 提示没有改写或隐去。

正式源码和异常副本中的相应文件 SHA-256 完全一致：

```text
trapentry.S  639770e6099b28225103e63e528adc41ce735889584a6b174098eb2d8db88405
trap.c       777cc84518e2287fb28fac9cee30619925ec4aa57fa9afb1c33eba1e4f1a96d6
```

最终副本四个故障地址与恢复地址：

| 场景 | 故障位置 | 恢复位置 |
| --- | --- | --- |
| illegal32 | `0xffffffffc0201f32` | `0xffffffffc0201f36` |
| illegal16 | `0xffffffffc02020da` | `0xffffffffc02020dc` |
| breakpoint16 | `0xffffffffc0202282` | `0xffffffffc0202284` |
| breakpoint32 | `0xffffffffc020242a` | `0xffffffffc020242e` |

这些是最终串口日志中探针标签位置，不是新的手动 GDB 截图。原因码实际读取的手动证据仍使用早期截图，并明确其参考入口依赖。

## 本地日志

- `final-grade-20261106.log`
- `final-build-20261106.log`
- `final-qemu-20261106.log`
- `final-exception-prepare-20261106.log`
- `final-exception-build-20261106.log`
- `final-exception-qemu-20261106.log`

日志与新记录均位于本地忽略目录，本次没有提交或推送。报告只引用真实原图和可核验的文字结果。
