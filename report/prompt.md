# Lab 1 提示词汇总

成员：2410673-韩羽宸。AI 工具：OpenAI ChatGPT；底层模型：GPT-6。

下面结合手册和源码，将成员1构建与输出部分的问题整理为四段式提示词。结果部分依据代码分析和实际运行记录填写。

## 提示词 1：OpenSBI、ELF 与内核镜像的关系

### 目的

理解手册中几个文件和运行阶段的区别，明确内核是怎样进入模拟内存并开始执行的。

### 完整提示词

```text
[PROMPT]
手册把 OpenSBI、ELF 和 bin 分开介绍了，我想把它们放到一次完整启动中
理解。结合这个工程讲一下：bin/kernel 和 bin/ucore.img 分别是什么？
为什么生成了 ELF 还要用 objcopy 转换？QEMU 和 OpenSBI 各负责哪一步？

[RELY]
Makefile 将目标文件链接为 bin/kernel，再通过 objcopy -O binary
生成 bin/ucore.img。链接基址为 0x80200000，QEMU 使用 virt 平台和
默认 OpenSBI 固件。file 检查显示 kernel 是 RISC-V ELF，ucore.img 是 data。

[GUARANTEE]
请说明两种文件保存的信息、装入方式，以及 QEMU、固件和内核的关系。
以当前工程为准，不修改代码。

[SPECIFICATION]
从构建文件一直讲到内核开始执行，区分装入内存和交接控制权。
解释原始镜像没有 ELF 头部后，加载地址和入口由什么确定。
再说明 .bss 是否需要把全部零字节存进文件，为什么 kern_init 还要清零。
```

### 结果与后续调整

ELF 包含入口、装入及调试信息，原始镜像保留转换后的二进制内容，需要与启动配置配合。QEMU 根据参数装入固件和内核，OpenSBI 初始化底层环境并向 S 模式内核交接控制权。`.bss` 通常只在 ELF 中描述所需空间，内核启动后通过 `memset` 完成清零。

## 提示词 2：链接脚本怎样确定内存布局

### 目的

将手册中的链接脚本概念对应到实际符号和地址，理解入口、段布局与初始化之间的关系。

### 完整提示词

```text
[PROMPT]
结合手册和 tools/kernel.ld，按脚本顺序讲一下内核的内存布局。
OUTPUT_ARCH、ENTRY 和 BASE_ADDRESS 分别控制什么？ENTRY(kern_entry)
和把 .text.kern_entry 放在前面有什么区别，为什么两者都要考虑？

[RELY]
脚本指定 RISC-V 架构，入口为 kern_entry，基址为 0x80200000。
输出段依次包括 .text、.rodata、.data、.sdata 和 .bss。
脚本使用 ALIGN(0x1000)，并定义 etext、edata、end。
kern_init 执行 memset(edata, 0, end - edata)。

[GUARANTEE]
解释位置计数器、段收集、对齐和边界符号，并将它们与启动初始化对应。
这里只分析已有脚本，不修改入口代码。

[SPECIFICATION]
说明 edata 到 end 的清零范围为什么由链接器提供。
区分 ELF 中声明的入口与原始镜像启动时实际使用的地址。
我执行 ld --verbose 看到的是 x86-64 和 ENTRY(_start)，请解释这与
本实验 RISC-V 链接脚本的关系，避免混淆宿主机和目标机工具链。
```

### 结果与后续调整

`ENTRY` 声明 ELF 入口，段收集顺序影响代码在镜像中的实际位置。当前入口地址为 `0x80200000`，与内核基址一致。`ALIGN(0x1000)` 表示按 4096 字节对齐，`edata`、`end` 确定启动清零范围。系统 `ld --verbose` 展示宿主机默认脚本，内核实际由 RISC-V 链接器使用 `tools/kernel.ld` 构建。

## 提示词 3：从 cprintf 追踪到 SBI 字符输出

### 目的

理解手册中“从 SBI 到 stdio”的封装过程，以及格式化输出与特权级切换的关系。

### 完整提示词

```text
[PROMPT]
从 kern_init 中的 cprintf("%s\n\n", message) 开始，沿实际代码追踪
这条启动消息是怎么输出的。请把格式解析、字符回调和 SBI 调用连起来，
不要只分别介绍函数。

[RELY]
相关文件：kern/libs/stdio.c、libs/printfmt.c、kern/driver/console.c、
libs/sbi.c，以及项目中的 stdarg.h。
相关接口：cprintf、vcprintf、vprintfmt、cputch、cons_putc、
sbi_console_putchar 和 sbi_call。
本工程使用传统 SBI 字符输出服务，服务编号为 1。

[GUARANTEE]
给出完整调用链，解释可变参数如何传递，以及 vprintfmt 为什么接收
一个字符输出回调。选取关键代码说明，不重写已有函数。

[SPECIFICATION]
解释 sbi_call 中 x17、x10、x11、x12 与 a7、a0、a1、a2 的对应关系，
说明服务编号、参数和返回值放在哪里，ecall 如何请求 OpenSBI 服务。
最后说明这种 S 模式内核调用 M 模式固件的过程，与用户程序向操作系统
发起系统调用有什么联系和区别。
```

### 结果与后续调整

`cprintf` 组织可变参数，`vcprintf` 将参数交给 `vprintfmt`。格式解析通过 `cputch` 回调逐字符输出，随后经过控制台和 SBI 封装。`sbi_call` 将服务编号放入 `a7`，参数放入 `a0` 等寄存器，再执行 `ecall` 请求固件服务。格式解析与字符输出分开，便于保留上层接口并调整底层实现。

## 提示词 4：Makefile 的完整构建过程

### 目的

理解手册中 Just make it 对应的实际构建规则，而不只停留在执行 make 命令。

### 完整提示词

```text
[PROMPT]
结合 Makefile 和 tools/function.mk，说明执行 make 后怎样从 .c、.S
源码得到 bin/kernel 和 bin/ucore.img。请按编译、链接、镜像转换的
顺序分析，并说明 make 怎么知道哪些文件需要重新构建。

[RELY]
工具链前缀为 riscv64-unknown-elf-，Makefile 包含 function.mk。
KOBJS 汇集内核和基础库的目标文件，链接命令使用 -T tools/kernel.ld。
镜像由 objcopy --strip-all -O binary 生成。
编译选项包括 -g、-nostdinc、-ffunction-sections、-fdata-sections；
链接选项包括 -nostdlib 和 --gc-sections。

[GUARANTEE]
说明源码收集、目标文件规则、依赖文件、链接和镜像生成之间的关系。
解释上述关键选项，不修改构建规则。

[SPECIFICATION]
结合 listf、add_files、cc_template 和 KOBJS 解释规则如何组织。
区分交叉编译与在宿主机上编译普通程序，说明内核为什么使用自己的
头文件和基础函数。解释保留 ELF 调试信息与生成原始镜像是否矛盾，
以及生成的 kernel.asm、kernel.sym 可以帮助分析哪些内容。
```

### 结果与后续调整

`function.mk` 组织源码列表、编译和依赖规则，Make 根据依赖关系和更新时间决定是否重建。目标文件通过链接脚本生成 ELF，再转换为原始镜像。ELF 仍保留调试信息，镜像转换并不覆盖原 ELF；链接阶段还生成反汇编和符号信息，便于对照机器指令、源码与符号地址。实际 `make` 已完成编译、链接和镜像生成。

## 提示词 5：启动失败的定位与参数修复

### 目的

根据实际固件输出检查启动交接，修复内核没有执行的问题。

### 完整提示词

```text
[PROMPT]
make 已经成功，但 make qemu 只打印了 OpenSBI 信息，没有出现内核消息。
请结合手册、Makefile 和链接脚本判断问题在哪个阶段。
原来的 loader 参数和 -kernel 参数有什么区别？确认原因后，直接修改
Makefile 的 qemu 规则，说明为什么这样改，以及怎么判断修复成功。

[RELY]
环境：Ubuntu 22.04、QEMU 7.0.0、OpenSBI 1.0。
原始参数：-device loader,file=$(UCOREIMG),addr=0x80200000。
异常输出：Domain0 Next Address 为 0，未出现内核启动消息。
镜像：bin/ucore.img；链接基址和当前入口地址为 0x80200000。
kern_init 打印“(THU.CST) os is loading ...”后进入循环。

[GUARANTEE]
只修改 Makefile 的 qemu 目标，将加载参数改为 -kernel $(UCOREIMG)。
保留编译、链接规则、debug 目标和内核源码，给出修改差异及复验依据。

[SPECIFICATION]
Pre-Condition：bin/kernel 和 bin/ucore.img 已成功生成。
Post-Condition：OpenSBI 下一阶段地址为 0x80200000，模式为 S-mode，
随后输出内核消息。
Case 1：只有固件信息时，检查下一阶段地址，不能直接判定内核已启动。
Case 2：内核输出后停在循环中，符合当前 kern_init 的预期行为。
Requirements：将结论限定在当前环境，区分装入地址与启动入口配置。
```

### 结果与后续调整

首次运行中，镜像构建成功，但 OpenSBI 的下一阶段地址为 0，说明启动交接需要继续检查。原来的 loader 参数指定了镜像装入地址，在当前默认固件环境中没有正确配置下一阶段入口。

将正常运行目标改为 `-kernel $(UCOREIMG)` 后，重新执行 `make qemu`，下一阶段地址变为 `0x0000000080200000`，模式为 `S-mode`，随后出现内核消息。输出后保持循环，与源码相符。实际改动只有这一行参数替换。





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
