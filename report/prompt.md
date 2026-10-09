# Lab 1 提示词汇总

成员：2410673-韩羽宸。AI 工具：OpenAI ChatGPT；底层模型：GPT-6。

以下按实验内容整理提问及分析结果。

## 提示词 1：OpenSBI、ELF 和内核镜像

### 目的

理解手册中 OpenSBI、ELF 和 bin 的关系。

### 完整提示词

```text
结合手册讲一下，bin/kernel 和 bin/ucore.img 有什么区别？QEMU 和 OpenSBI 在启动时分别做什么？
```

### 结果与后续调整

`bin/kernel` 是链接得到的 ELF 文件，包含入口、装入和调试信息。`bin/ucore.img` 是通过 `objcopy -O binary` 转换得到的原始镜像，没有 ELF 头部，需要由启动配置确定如何装入和执行。

QEMU 根据参数将固件和镜像装入模拟内存，OpenSBI 在 M 模式完成初始化，再交给 S 模式内核执行。内核运行后仍可通过 SBI 请求字符输出。两者承担的工作不同，因此镜像已经装入内存，还需要确认控制权是否正确交给内核。

## 提示词 2：链接脚本与内存布局

### 目的

理解入口、段布局和启动清零之间的关系。

### 完整提示词

```text
帮我结合手册看一下 kernel.ld，入口地址和各段的位置是怎么确定的？edata、end 为什么能用来确定清零范围？
```

### 结果与后续调整

`OUTPUT_ARCH` 指定 RISC-V 架构，`ENTRY(kern_entry)` 声明 ELF 入口，位置计数器从 `0x80200000` 开始。入口代码优先收集到 `.text`，本次生成的入口地址也为 `0x80200000`。

后续段保存只读数据、已初始化数据和零初始化数据，`ALIGN(0x1000)` 表示按 4096 字节对齐。`edata` 在初始化数据之后，`end` 在 `.bss` 之后，因此 `memset(edata, 0, end - edata)` 可以清零这段区域。`.bss` 通常不在 ELF 中保存全部零字节，启动时仍需初始化。

另外，`ld --verbose` 显示的 x86-64 脚本属于宿主机；本实验使用的是 RISC-V 链接器和自定义的 `kernel.ld`。

## 提示词 3：SBI 输出链

### 目的

理解手册中从 SBI 到 stdio 的封装过程。

### 完整提示词

```text
kern_init 里的 cprintf 是怎么把启动消息输出到终端的？结合代码把它到 SBI 的调用过程串起来讲一下。
```

### 结果与后续调整

`cprintf` 收集可变参数，再通过 `vcprintf` 交给 `vprintfmt` 解析格式。解析出的字符由 `cputch` 回调输出，依次经过 `cons_putc`、`sbi_console_putchar` 和 `sbi_call`。

字符输出服务编号为 1，`sbi_call` 将编号放入 `a7`、参数放入 `a0` 等寄存器，通过 `ecall` 请求 OpenSBI 服务。上层负责格式解析，底层负责实际字符输出，两部分通过回调连接起来。

这里是 S 模式内核向 M 模式固件请求服务。它与系统调用都有通过规定接口进入更高特权级的特点，但系统调用通常是用户程序请求操作系统服务，两者的处理层次不同。

## 提示词 4：Makefile 构建过程

### 目的

理解手册中执行 make 后的编译、链接和镜像生成过程。

### 完整提示词

```text
执行 make 后，源码是怎么一步步变成 ucore.img 的？结合 Makefile 讲一下编译、链接和 objcopy 之间的关系。
```

### 结果与后续调整

`function.mk` 组织源码、编译和依赖规则，交叉编译器将 C、汇编源码转换为目标文件。`KOBJS` 汇集内核和基础库的目标文件，链接器按 `tools/kernel.ld` 生成 `bin/kernel`，再由 `objcopy` 转为 `bin/ucore.img`。

`-g` 使 ELF 保留调试信息，转换镜像不会覆盖原 ELF。`-nostdinc`、`-nostdlib` 使内核使用自己的头文件和基础实现。Make 根据依赖关系和文件更新时间决定是否重建，因此再次执行时不一定会重新编译所有文件。

实际执行 `make` 后，编译、链接和镜像生成均完成，文件检查确认两种产物已生成。

## 提示词 5：启动异常与修复

### 目的

根据实际输出定位内核没有启动的问题。

### 完整提示词

```text
make 成功了，但 make qemu 只停在 OpenSBI，Next Address 是 0。帮我看一下原因，修好 Makefile 的启动参数。
```

### 结果与后续调整

首次运行只有固件信息，没有内核消息。链接脚本中的内核入口为 `0x80200000`，而固件下一阶段地址为 0，因此需要检查镜像装入和启动交接的配置。

原来的 `-device loader` 指定了镜像装入地址，在当前默认固件环境中没有正确配置下一阶段入口。将正常运行目标改为 `-kernel $(UCOREIMG)` 后，下一阶段地址变为 `0x0000000080200000`，模式为 `S-mode`，随后出现 `(THU.CST) os is loading ...`。

修改只涉及一行启动参数。内核输出后进入循环，与 `kern_init` 的代码一致，这次停留属于正常运行状态。

**记录人：** 2410933-马禹翔。AI 工具：GitHub Copilot；底层模型：GPT-6。

## 提示词 6：使用 GDB 验证内核启动流程

### 目的

使用 QEMU GDB stub 观察 RISC-V 从复位向量、OpenSBI 到内核入口的控制流，并如实记录 GDB 可用性和验证结果。

### 完整提示词

```text
[PROMPT]
请结合 Makefile 的 debug 目标和
tools/kernel.ld，给出从 QEMU 复位到内核入口 0x80200000 的双终端调试步骤，
解释如何检查最初几条指令、单步进入 OpenSBI，并在内核入口断下。
回答最初指令位于什么地址、分别完成什么工作。

[RELY]
项目为 ucore 操作系统 Lab1。Makefile 的 debug 目标通过 QEMU 的 -S -s
暂停虚拟 CPU 并开放 localhost:1234，tools/kernel.ld 将入口 kern_entry
链接在 0x80200000。实验环境为 QEMU 4.1.1 和 gdb-multiarch 17.2。
实际 GDB 连接后初始 pc 为 0x1000；stepi 5 后 pc 为 0x80000000；
在 0x80200000 断下时符号为 kern_entry，第一条指令是 auipc sp,0x3；
单步后 pc 为 0x80200004，sp 为 0x80203000。

[GUARANTEE]
给出可复现的 GDB 命令和本次实际观察结果，区分复位向量、OpenSBI
固件入口和内核入口；仅记录真实执行并观察到的步骤，不虚构截图或输出。

[SPECIFICATION]
说明两终端的启动与连接方法：make debug、启动 RISC-V GDB、连接
localhost:1234、查看 $pc 和反汇编、单步五条复位指令、在 0x80200000
设断点并继续。回答 0x1000 处复位代码的职责、OpenSBI 的固件地址和
向内核交接的作用。记录断点处的内核入口指令，并单步验证栈指针变化。
将本次实际 GDB 输出与源码的启动逻辑对应，报告与提示词汇总中的结论保持一致。
```

### 结果与后续调整

使用 `make debug` 启动 QEMU，并通过 `gdb-multiarch bin/kernel` 连接 `localhost:1234`。GDB 初始显示 `pc=0x1000`；单步五条复位代码后到达 `0x80000000`；继续运行至 `0x80200000` 的 `kern_entry` 断点。入口首条指令为 `auipc sp,0x3`，单步后 `pc=0x80200004` 且 `sp=0x80203000`，随后反汇编显示跳转到 `kern_init`。因此实测确认复位代码位于 QEMU `virt` 的 `0x1000`，负责准备启动参数并进入 OpenSBI，之后固件再交接到内核入口。

## 提示词 7：复位代码跳转后的执行位置与文件来源

### 目的

根据已完成的 GDB 跟踪，记录加电后各阶段指令对应的存储位置。

### 完整提示词

```text
[PROMPT]
从加电开始，后续执行的指令存储在哪些文件中？先不介绍指令功能，
只指出地址和对应文件位置。

[RELY]
本次 QEMU virt 的复位入口地址为 0x1000；OpenSBI 固件基址为
0x80000000；内核入口为 0x80200000。QEMU 使用 -bios default，项目
链接脚本指定 ENTRY(kern_entry)，原始内核镜像为 bin/ucore.img。

[GUARANTEE]
只列出启动各阶段的运行地址和相应文件或源码位置，不介绍指令功能，
不修改源码或构建配置。

[SPECIFICATION]
按启动地址顺序区分 QEMU 复位 MROM、OpenSBI 固件镜像和内核镜像；
列出 QEMU 源码对应位置、当前 OpenSBI 固件文件、项目内核二进制镜像、
ELF 文件及入口汇编源码。结果部分仅记录这些位置。
```

### 结果与后续调整

位置如下：

1. `0x1000` 起：QEMU `virt` 的 MROM 内容，由 QEMU 生成，不是项目内的独立镜像文件；对应 QEMU 源码实现位于 `hw/riscv/boot.c`。
2. `0x80000000` 起：`/usr/local/share/qemu/opensbi-riscv64-virt-fw_jump.bin`。
3. `0x80200000` 起：项目内核镜像 `lab1/lab1/bin/ucore.img`；对应 ELF 为 `lab1/lab1/bin/kernel`，入口源码为 `lab1/lab1/kern/init/entry.S`，后续 C 代码位于 `lab1/lab1/kern/init/init.c`。

## 提示词 8：复位代码为何跳转到 0x80000000

### 目的

解释复位入口跳转到 OpenSBI 的原因，以及该目标地址与 QEMU `virt` 内存布局的关系。

### 完整提示词

```text
[PROMPT]
对于第一条指令，0x1000 是初始复位地址。执行到复位代码中的第一条
跳转指令后，为什么会跳转至 0x80000000？这一步的功能是什么？为什么
选择 0x80000000？请结合本次 GDB 观察和当前 QEMU 配置解释。

[RELY]
GDB 观察到 0x1000 开始的复位代码在 0x100c 执行 ld t0,24(t0)，
在 0x1010 执行 jr t0；单步五条指令后 pc 为 0x80000000。Makefile
使用 QEMU -machine virt -bios default，并把内核镜像加载到 0x80200000。
本机默认 OpenSBI 固件为 /usr/local/share/qemu/
opensbi-riscv64-virt-fw_jump.bin，运行时 Firmware Base 显示为 0x80000000。

[GUARANTEE]
解释复位桩到 OpenSBI 的交接目的，以及 0x80000000 对当前 virt 平台的
意义。区分平台配置与 RISC-V ISA 的固定要求，不把此地址说成所有 RISC-V
系统都必须使用的地址。

[SPECIFICATION]
结合 ld 与 jr 指令说明目标地址从何处取得；解释 OpenSBI 接手后的启动
阶段，以及它如何最终交接到 0x80200000 的内核。说明 QEMU virt 的 RAM
起始地址、默认固件加载位置与 0x80000000 的关系，并指出其他平台或固件
配置可能使用不同地址。将答案追加在本文件的新提示词条目中，不修改报告。
```

### 结果与后续调整

`0x100c` 的 `ld t0,24(t0)` 从复位 ROM 中 `0x1018` 的数据槽读取下一阶段地址，`0x1010` 的 `jr t0` 随后跳到该地址。本次 QEMU `virt` 配置下该值为 `0x80000000`，即默认 OpenSBI 固件基址。交接的作用是从 QEMU 的最小复位桩进入负责平台初始化的 M 模式固件；OpenSBI 完成准备后再通过 `mret` 将控制权交给 `0x80200000` 的内核入口。

`0x80000000` 并非 RISC-V 指令集规定的统一复位目标，而是当前机器型号和固件布局决定的地址：QEMU `virt` 的 RAM 从此处开始，`-bios default` 在此加载 OpenSBI。若更换硬件平台或固件配置，目标地址可能不同。

## 提示词 9：确认固件地址是否固定及跳转作用

### 目的

确认 `0x80000000` 是否为平台相关地址，并准确区分复位跳转、固件初始化和内核装载。

### 完整提示词

```text
[PROMPT]
所以，0x80000000 是由 QEMU 的固件决定的，而非绝对固定地址？
这一步的功能是加载 M 模式固件以进行内核初始化吗？请结合当前启动配置
说明哪些理解准确，哪些需要区分。

[RELY]
当前使用 QEMU -machine virt -bios default。复位代码由 0x1000 开始，
在 0x1010 执行 jr t0 并跳到 OpenSBI 基址 0x80000000；内核镜像由
Makefile 的 -device loader 加载到 0x80200000。OpenSBI 最终通过 mret
把控制权交给内核入口。

[GUARANTEE]
说明 0x80000000 是本次平台和固件配置下的地址，不是 RISC-V ISA 对
所有机器规定的绝对地址。区分“跳转到已装入的固件”和“装载内核镜像”，
不要把复位跳转本身描述为加载固件或内核。

[SPECIFICATION]
简要说明 QEMU virt、默认 OpenSBI 固件基址与复位代码目标之间的关系；
说明 OpenSBI 的 M 模式初始化及其向 0x80200000 内核入口的控制权交接；
指出当前内核镜像由 QEMU loader 装入。将澄清结果追加在本文件，不改报告。
```

### 结果与后续调整

理解基本正确：`0x80000000` 不是 RISC-V ISA 规定的固定地址，而是本次 QEMU `virt` 和默认 OpenSBI 固件布局使用的基址。需要修正的是“这一步加载固件”这一说法：QEMU 启动时已把 OpenSBI 固件放在该地址，复位代码的 `jr t0` 只是跳转到它。OpenSBI 随后进行 M 模式平台初始化并通过 `mret` 交接到 `0x80200000` 的内核；本次内核镜像由 QEMU 的 `-device loader` 装入，不是由这条跳转指令加载。

## 提示词 10：OpenSBI 向内核交接的含义

### 目的

区分内核镜像装入内存、OpenSBI 移交执行权和内核完成初始初始化这几个阶段。

### 完整提示词

```text
[PROMPT]
对于第二次跳转，即跳转至 0x80200000，这一步的功能是不是把控制权交给
内核？相当于操作系统内核初步加载完成？

[RELY]
Makefile 使用 -device loader,file=$(UCOREIMG),addr=0x80200000 将内核镜像
放置到入口地址。OpenSBI 最终通过 mret 进入该地址。entry.S 的 kern_entry
设置 sp 后 tail kern_init；kern_init 清零 edata 到 end 的区域并输出启动信息。

[GUARANTEE]
区分内核镜像的装入、CPU 控制权移交、内核启动初始化完成。不要把 OpenSBI
的 mret 描述为加载镜像，也不要把进入 kern_entry 等同于全部内核初始化结束。

[SPECIFICATION]
说明在当前 QEMU 配置下，内核镜像何时由 loader 放入内存；OpenSBI 如何把
执行流交给 0x80200000 的 kern_entry；入口和 kern_init 随后完成哪些初始工作。
给出“内核初步加载完成”这一表述何时可用、有哪些限定，并将结果追加在本文件。
```

### 结果与后续调整

可以说 OpenSBI 在 `0x80200000` 把 CPU 控制权交给内核，内核开始执行；但严格来说，这一步不是加载镜像。当前 QEMU 的 `-device loader` 已在启动执行前把 `bin/ucore.img` 放到 `0x80200000`。`mret` 完成的是从固件到内核入口的控制权移交。随后 `kern_entry` 设置内核栈并跳转到 `kern_init`，`kern_init` 清零 `.bss` 范围并输出启动信息。因此可以称内核镜像已装入且内核开始启动，但进入入口并不代表所有内核初始化都已完成。

## 提示词 11：kern_init 的 cprintf 与 SBI 调用

### 目的

确认 `kern_init`、格式化输出函数、`ecall` 与 M 模式 SBI 服务之间的关系。

### 完整提示词

```text
[PROMPT]
在 kern/init/init.c 中，kern_init() 是否是加载一个原始的 cprintf 函数，
以提供简单的输出交互接口？这个输出接口是否通过 ecall 提权到 M 模式，
再调用 SBI 服务实现？请沿代码调用链解释，并指出说法中需要修正的地方。

[RELY]
kern_init() 调用 cprintf 输出启动字符串。cprintf 和 vcprintf 位于
kern/libs/stdio.c；字符回调最终调用 kern/driver/console.c 中的 cons_putc；
libs/sbi.c 定义 SBI_CONSOLE_PUTCHAR 和 sbi_call，后者通过 ecall 发起 SBI 调用。

[GUARANTEE]
依据实际代码解释调用链，区分函数调用、格式化输出、SBI 调用和特权级陷入。
不要把 kern_init 描述为加载 cprintf，也不要把 cprintf 的输出功能称作输入交互。
说明 ecall 在当前 SBI 调用场景中的处理者。

[SPECIFICATION]
按 kern_init → cprintf → vcprintf/vprintfmt → 字符输出回调 → cons_putc
→ sbi_console_putchar → sbi_call → ecall → OpenSBI 服务的顺序说明。指出
sbi_call 使用 x17/a7 传 SBI 服务号、x10/a0 等寄存器传参数，服务号
SBI_CONSOLE_PUTCHAR 为 1。说明内核在 S 模式运行，当前 SBI ecall 由
M 模式 OpenSBI 处理；澄清 ecall 是受控陷入而非 cprintf 自行提权。
将结论追加在本文件，不修改源代码或报告。
```

### 结果与后续调整

`kern_init` 并没有加载 `cprintf`；它调用已经编译并链接进内核的 `cprintf` 输出字符串。调用链为 `kern_init → cprintf → vcprintf → vprintfmt → cputch → cons_putc → sbi_console_putchar → sbi_call → ecall → OpenSBI`。`cprintf` 负责格式化输出，不是输入交互接口。

内核在 S 模式执行 `ecall` 后，当前 SBI 配置下由 M 模式 OpenSBI 接收并处理控制台输出服务。`sbi_call` 把服务号放入 `x17/a7`、参数放入 `x10/a0` 等寄存器；本工程中 `SBI_CONSOLE_PUTCHAR` 的值为 1。准确地说，是 SBI 调用通过 `ecall` 进入固件处理，不是 `cprintf` 自身提权。
