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
