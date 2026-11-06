# Lab3 提示词汇总

成员：**2410673 韩羽宸、2410933 马禹翔、2410936 林子媛**。

本文件按成员和任务汇总 11 组提示词，与 [Lab3 小组实验报告](report.md) 配套。每组正文依次采用 **[PROMPT] 任务说明、[RELY] 上下文信息、[GUARANTEE] 交付清单、[SPECIFICATION] 行为规格**，之后另列实际结果或理解修正，不将结果混入生成前的要求。

提示词中的路径、版本、操作限制和提交要求属于原实施阶段的上下文，不是本次整理要重复执行的命令。本文件仅摘录 `report.md` 中已有的提示词及对应说明；复现稿按同一四段式排版，不补造报告未记载的交流或执行结果。原始个人报告与马禹翔的独立提示词文件保留供核对。

## 提示词索引

| 编号 | 负责人 | 任务 | 对应报告 |
| --- | --- | --- | --- |
| [HYC-P1](#hyc-p1) | 2410673 韩羽宸 | 实现 Trap 入口与上下文恢复 | 4.1 / 5.2 / 5.5 |
| [HYC-P2](#hyc-p2) | 2410673 韩羽宸 | 验证真实入口、处理后的帧与返回状态 | 4.1 / 5.2 |
| [HYC-P3](#hyc-p3) | 2410673 韩羽宸 | 现场调试与截图取证 | 4.1 / 5.2 |
| [MYX-P1](#myx-p1) | 2410933 马禹翔 | 中断原因分类与兜底处理 | 4.2 / 5.3 |
| [MYX-P2](#myx-p2) | 2410933 马禹翔 | 持续时钟中断与计数输出 | 4.2 / 5.3 |
| [MYX-P3](#myx-p3) | 2410933 马禹翔 | 关机条件与整体集成验证 | 4.2 / 5.1 |
| [MYX-P4](#myx-p4) | 2410933 马禹翔 | 分析中断关闭与状态保存 | 4.4 |
| [MYX-P5](#myx-p5) | 2410933 马禹翔 | 分析中断恢复与条件开启 | 4.4 |
| [LZY-P1](#lzy-p1) | 2410936 林子媛 | 生成异常处理函数 | 4.3 / 5.4 |
| [LZY-P2](#lzy-p2) | 2410936 林子媛 | 独立异常自测、真实指令探针与返回检查 | 4.3 / 5.4 |
| [LZY-P3](#lzy-p3) | 2410936 林子媛 | stvec 模式编码的向后兼容 | 4.5 |

---

**2410673 韩羽宸：** 三组提示词按报告 4.1 原文摘录，属于实际需求、源码和验证方案的规范化记录，不是逐字聊天输入；实现、验证和现场观察分开记录。

<a id="hyc-p1"></a>

## HYC-P1：实现 Trap 入口与上下文恢复

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

我是 2410673 韩羽宸。请依据 Lab3 手册、分工和实际源码，只完成
code/kern/trap/trapentry.S 中 RESTORE_ALL 与 __alltraps 两处功能一。
先核对正式 code 与已拉取的 lab3 分支，保留队友已完成的异常函数；
不把同步来的队友代码计作本人实现。不要修改 SAVE_ALL、公共结构体、
trap() 签名、原 __trapret、评分脚本或队友负责区域。
输出实际代码、关键顺序的解释和差异自查；验证交给 HYC-P2。
完成检查后仅将本人代码本地提交，中文说明以“韩羽宸代码部分：”开头，
不执行 git push。

### [RELY] 上下文信息

项目是 RV64 ucore，REGBYTES=8，LOAD/STORE 使用工程已有宏。
SAVE_ALL 先把原 sp 存入 sscratch，再分配 36 个槽；保存完 GPR 后，
通过 csrrw 取回原 sp 并将 sscratch 清零，原 sp 写入第 2 槽。
前 32 槽按 x0—x31 排列，32/33/34/35 槽为 status/epc/badvaddr/cause，
对应保存的 sstatus/sepc/stval/scause；一帧为 288 字节。
trap.h 已定义 struct pushregs、struct trapframe，C 接口是：
void trap(struct trapframe *tf);
trap() 分发给队友的 interrupt_handler()、exception_handler()；
C 可以修改保存帧，返回时应使用处理后的内容。现有 __trapret 紧接
入口调用位置，执行 RESTORE_ALL 与 sret。本轮实际验证为 S 态内核，
不要额外设计用户态换栈、调度或扩展寄存器上下文。

### [GUARANTEE] 交付清单

1. 两处完整实现及保留 lab3 function1、标注 2410673 的源码。
2. 说明帧槽位、a0 传参、CSR 中转、sp 最后恢复和 sret 的衔接。
3. 差异清单，确认保存宏、结构体、返回标签和队友实现保持原样。
4. 验证后仅含本人代码的本地提交记录，列明基线、文件和推送状态。

### [SPECIFICATION] 行为规格

**Pre-Condition：** Trap 已进入 __alltraps；当前内核栈有效、空间足够且
按 ABI 对齐；已有 SAVE_ALL 将现场存为与 C 结构体一致的帧。

**Post-Condition：** 以 a0=sp 调用 trap；C 返回时 sp 仍指向同一帧首。
RESTORE_ALL 从处理后的第 32/33 槽装入 sstatus/sepc，恢复 x1、x3—x31，
最后从第 2 槽恢复 x2；之后顺序执行 sret，回到帧指定的执行位置。

**Case 1：** 临时借用 s0/s1 搬运 CSR 后，仍须从各自槽位恢复其原值。

**Case 2：** C 改写 epc 或保存的 a0 时，恢复必须让这些修改生效，
不能用另一份旧现场覆盖；未修改字段保持对应值。

**Case 3：** 原全局 SIE 为 0 或 1，均按保存的状态和 sret 语义返回；
不在恢复宏强行开中断、清 SPP 或统一推进 EPC。

**Requirements：** x0 无需加载；本返回路径无需写回 stval/scause。
sp 在最后一次帧读取前一直作为帧基址；恢复 x2 后不再加 288。
288 字节保持原栈的 16 字节对齐，但不能修复输入栈本身未对齐。

### 结果与后续调整

两处实现已完成，`SAVE_ALL` 与原 `__trapret` 保留；相对基线 `9ceb1ac`，一个文件新增 44 行、删除 2 行，其中新增实际汇编指令 37 行。本地提交为 `e4f5256`，未推送；实际差异及记录见 5.2、5.5。

<a id="hyc-p2"></a>

## HYC-P2：验证真实入口、处理后的帧与返回状态

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

为本人功能一建立可复现验证，检查“保存—C 处理—恢复—继续执行”
整条路径。先复用队友的异常探针，再补充帧参数、寄存器、栈、EPC
和返回状态断言。所有驱动、观察函数、构建副本及日志放在附加材料；
正式 code 和评分脚本保持不变，不修改断言来获得通过。
只打印异常不能作为恢复成功的依据；必须检查 sret 之后的实际状态。

### [RELY] 上下文信息

使用 HYC-P1 的真实 trapentry.S、trap.h 和队友 exception_handler()。
已有探针覆盖 16/32 位非法指令和 16/32 位断点，设置整数寄存器标记值，
返回后比较 GPR 和栈；异常函数负责推进 EPC，恢复宏负责使其生效。
正式 interrupt_handler() 尚无定义。测试可在独立副本加入“意外中断
立即 panic”的失败桩，于第二次 idt_init() 后、clock_init() 前运行，
完成后使用现有 SBI 服务退出；不能用桩代替队友的时钟实现。
已有测试框架可能替换参考入口，必须核对实际使用的汇编而非相信提示语。

### [GUARANTEE] 交付清单

1. 两种独立副本：异常联调、上下文状态检查；保留真实正式汇编。
2. 入口及处理后观察断言、ABI 安全的汇编探针、复现命令。
3. 构建日志、串口原始输出、QEMU 实际退出码、被测源码哈希和快照。
4. 按“初态—操作—预期—实际—判定”记录结果，说明联调与隔离边界。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 明确被测源码和副本来源；RV64 工具链可用，栈有效。
比较正式与副本 trapentry.S 的内容及哈希；不得使用 reference-entry.S。

**Post-Condition：** 真实触发后能继续执行，帧参数、寄存器、原栈、EPC
和返回状态均满足断言；成功退出、失败终止和未执行分别记录。

**Case 1：** 四种真实异常分别检查恢复标签、2/4 字节 EPC 更新及返回后的
寄存器值；保留半字对齐的 32 位指令场景，不只测四字节对齐。

**Case 2：** 在副本 trap() 前后观察同一 tf，核对大小、字段偏移、16 字节
对齐、tf+sizeof(*tf)==保存的原 sp、原因、地址和寄存器标记值。

**Case 3：** 将 sie=0 屏蔽具体中断源，分别令全局 SIE 为 0/1，再触发同步
异常。区分进入前、帧中与 sret 后的 SPP/SPIE/SIE；重复运行检查栈漂移。

**Case 4：** C 有意修改 tf->gpr.a0，返回后检查实际值，证明恢复使用处理后
的现场；这项结果与未修改寄存器保持不变分别判断。

**Requirements：** 探针先保护自身 ABI 状态，避免把测试代码破坏现场误判
为内核错误。直接调用异常函数的合成帧不算真实 Trap 返回验证。
有 PASS 但外层退出非零时，保留原记录并单独核验退出状态，不先改汇编。
隔离验证不等于时钟路径或 make grade 通过；测试次数由实际执行统计。

### 结果与后续调整

两种副本均使用本人正式汇编，四类真实异常返回通过。上下文测试在两种 SIE 下各运行 128 次普通探针，再各运行一次修改保存 `a0` 的断点，共 258 次；单独复核及报告复现时 QEMU 退出码均为 0。原始输出、断言依据和未覆盖项见 5.2，不将此结果写成全组评分通过。

<a id="hyc-p3"></a>

## HYC-P3：现场调试与截图取证

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

请按 Lab2 的带做方式，带我现场观察功能一并补齐报告截图。
一次只给当前终端所需的少量命令，说明观察目的和预期停点，等我提供
实际输出后继续。优先复用已有验证和工具，避免无关安装、重复构建，
不要一次给完全部调试流程。先取得真实画面，再补报告中的图与说明。

### [RELY] 上下文信息

当前报告 5.2 已有异常联调和上下文验证日志，但本人现场截图尚未取得。
证据目录有被测汇编快照、prepare_hyc.py、测试源与复现说明。
QEMU 运行 bin/ucore.img，RISC-V GDB 加载带符号的 bin/kernel；
两者应来自同一构建。关键位置为 __alltraps、SAVE_ALL 后的调用点、
trap()、__trapret 及恢复末尾；实际地址从当前符号和反汇编读取。

### [GUARANTEE] 交付清单

1. 精简的现场步骤和每一步的输出解释，必要时区分 QEMU 与 GDB 终端。
2. 三类原图：真实异常返回、上下文状态汇总、帧与入口参数关系。
3. 原图放 report/images/2410673/，使用相对链接和解释性图注；
日志、补充命令保存在附加材料。未取得的图继续标为待补。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 先核对实际工具、被测版本和会话状态，不干扰已有进程。

**Post-Condition：** 每张图能对应真实命令、测试对象与结论，正文不重复
堆放完整日志；截图由本人现场取得，助手核对、命名、归档。

**Case 1：** 先运行两个已有副本，分别展示四类真实返回与状态汇总、退出码；
退出码来自 shell，不能伪写成内核输出。

**Case 2：** GDB 在异常前记录原 sp，保存后检查帧首、原 sp 槽及状态/EPC，
在调用前核对 a0==sp，再观察处理后 EPC 和恢复末尾的原栈关系。
断点应避开改变观察结论的干扰；需要时用硬件断点，原理不明时先解释。

**Case 3：** 停点或数值不符时先看当前指令、符号和寄存器，再决定下一步；
不通过写寄存器、改内存或填入预期数值制造成功结果。

**Requirements：** 只截图与任务结论有关的过程，不重复放环境版本图。
不得绘制终端截图或手输 PASS；现场观察尚未执行时不能写成已验证。

### 结果与后续调整

已按输出逐步完成异常联调与 GDB 现场观察。按本人“只挑重点”的要求，将原三类截图精简为两张：四类真实异常返回，以及入口帧与恢复后的 PC/SP；上下文状态检查保留日志。原图均已归档并插入 5.2，未逐条单步的部分不记为已观察。

**2410933 马禹翔：** 五组提示词按报告 4.2、4.4 原文摘录，统一编号 MYX-P1—P5，并注明原 V1-A/B/C、V2-A/B 编号；仅统一标题层级和版式。

<a id="myx-p1"></a>

## MYX-P1：中断原因分类与兜底处理

**负责人：2410933 马禹翔**

报告原编号：**Prompt V1-A**。

### [PROMPT] 任务说明

**任务**：修改 `code/kern/trap/trap.c` 中本人负责的 `interrupt_handler()`，先完成中断原因码提取、已知中断分类提示和未知中断兜底。

**操作要求**：必须修改实际文件。保留 Trap frame、`trap_dispatch()`、队友实现和评分脚本，只修改本人函数区域及必要头文件。保留 Lab3 注释并将本人标记改为 `2410933`。

**输出要求**：使用工程中的真实 IRQ 宏和已有打印接口，不修改测试条件。完成后解释为什么必须去掉 `scause` 最高位。

### [RELY] 上下文信息

```c
#define IRQ_U_SOFT   0
#define IRQ_S_SOFT   1
#define IRQ_H_SOFT   2
#define IRQ_M_SOFT   3
#define IRQ_U_TIMER  4
#define IRQ_S_TIMER  5
#define IRQ_H_TIMER  6
#define IRQ_M_TIMER  7
#define IRQ_U_EXT    8
#define IRQ_S_EXT    9
#define IRQ_H_EXT    10
#define IRQ_M_EXT    11
#define IRQ_COP      12
#define IRQ_HOST     13

void print_trapframe(struct trapframe *tf);
```

`trap_dispatch()` 使用 `((intptr_t)tf->cause < 0)` 判断中断。`tf->cause` 最高位是中断标志，低位才是原因码。评分要求 S 模式软件中断输出包含 `Supervisor software interrupt`，未知原因 15 输出包含 `Unknown interrupt: cause 15`。

### [GUARANTEE] 交付清单

必须实现或修改：

```c
void interrupt_handler(struct trapframe *tf);
```

本阶段完成原因码提取、已知类型分支和 default 兜底；时钟分支的具体计数行为由下一段规格补充。

### [SPECIFICATION] 行为规格

#### interrupt_handler：分类部分

**Pre-Condition**:

- `tf` 指向有效 Trap frame。
- 调用者已确认本次 Trap 是中断。

**Post-Condition**:

- 使用去除最高位后的低位原因码进行分类。
- 已知原因码输出可辨识的英文类型名称。
- 未知原因码先输出 `Unknown interrupt: cause <原因码>`，再调用 `print_trapframe(tf)`。

**Requirements**:

- S 模式软件中断输出必须包含评分要求的固定文本。
- 不得直接以包含最高中断标志位的完整 `tf->cause` 与小整数 IRQ 宏比较。

### 结果与后续调整

实现前的 `undefined reference to interrupt_handler` 是起始代码只有声明、没有本人实现造成的正常基线，不能写成 AI 生成 bug。之后把单段 prompt 重构为 V1-A/B/C，是为了使分类、计时和集成可以分别核验，也不属于代码修复迭代。

- `Supervisor software interrupt`：说明函数正确去掉中断标志并识别 S 模式软件中断；
- `Unknown interrupt: cause 15`：说明 default 分支使用低位原因码；

<a id="myx-p2"></a>

## MYX-P2：持续时钟中断与计数输出

**负责人：2410933 马禹翔**

报告原编号：**Prompt V1-B**。

### [PROMPT] 任务说明

**任务**：完善 `interrupt_handler()` 的 `IRQ_S_TIMER` 分支，使 OpenSBI 的单次时钟事件形成持续时钟中断，并完成计数与周期输出。

**操作要求**：直接修改实际函数，只补充时钟分支，不改变其他中断分类、Trap 返回流程和测试代码。

**输出要求**：说明时钟事件续设、`ticks` 更新和 `TICK_NUM` 输出三者的先后关系，并以持续达到 200 ticks 为验收目标。

### [RELY] 上下文信息

```c
extern volatile size_t ticks;
void clock_set_next_event(void);

#define TICK_NUM 100
static void print_ticks(void);
```

`clock_set_next_event()` 调用 OpenSBI 设置下一次单次事件。评分脚本等待 `ticks >= 200`，并检查 `100 ticks` 和 `ticks=200`。

### [GUARANTEE] 交付清单

必须完善：

```c
void interrupt_handler(struct trapframe *tf);
```

本阶段只规定 `IRQ_S_TIMER` 的持续触发、计数和周期输出行为。

### [SPECIFICATION] 行为规格

#### interrupt_handler：S 模式时钟分支

**Pre-Condition**:

- 低位原因码等于 `IRQ_S_TIMER`。
- `clock_init()` 已使能时钟中断并设置第一次时钟事件。

**Post-Condition**:

- 已安排下一次时钟事件。
- `ticks` 对本次时钟中断恰好增加一次。
- 每达到 `TICK_NUM` 的整数倍输出一次统计信息。

**Requirements**:

- 每次时钟中断都必须调用 `clock_set_next_event()`，否则系统只能收到一次事件。
- 不得直接写入评分通过文本代替真实计时过程。
- 系统必须能持续运行到评分所需的 200 ticks。

### 结果与后续调整

第二张图显示总共十次 `100 ticks`，随后 QEMU 退出并重新出现 shell 提示符。这与 `SHUTDOWN_AFTER=10`、`TICK_NUM=100` 一致，证明系统持续接收至少 1000 次时钟中断，并在第十个统计周期调用关机接口。

<a id="myx-p3"></a>

## MYX-P3：关机条件与整体集成验证

**负责人：2410933 马禹翔**

报告原编号：**Prompt V1-C**。

### [PROMPT] 任务说明

**任务**：为完成后的中断处理模块补充课程要求的关机条件，并进行编译、运行和完整评分验证。

**操作要求**：只增加必要的 `sbi_shutdown()` 依赖和模块内部阈值，不修改公开接口、队友代码或评分脚本。

**输出要求**：关机时机必须晚于 200 ticks。依次执行编译和正式评分，记录真实结果与未覆盖边界。

### [RELY] 上下文信息

```c
void sbi_shutdown(void);
#define TICK_NUM 100
```

实验要求累计输出达到规定次数后关机；当前起始代码没有提供 `SHUTDOWN_AFTER`。采用 10 个统计周期作为模块内部阈值，即 1000 ticks 后关机。

### [GUARANTEE] 交付清单

必须完成：

```c
void interrupt_handler(struct trapframe *tf);
```

允许增加 `#include <sbi.h>` 和模块内部关机阈值宏。

### [SPECIFICATION] 行为规格

#### interrupt_handler：关机与集成

**Pre-Condition**:

- 时钟续设、计数和周期输出已经实现。

**Post-Condition**:

- 每完成一个 `TICK_NUM` 周期可判断累计周期数。
- 累计 10 个统计周期后调用 `sbi_shutdown()`。
- 编译、异常/兜底测试和持续时钟测试均能通过。

**Requirements**:

- 关机条件不能早于 200 ticks。
- 不得修改测试以迁就实现。
- 使用 `make clean && make` 和 `make grade` 验证真实代码。

### 结果与后续调整

本模块代码经历一次实现：先写规格，再修改真实 `trap.c`，随后执行 `make clean && make` 和 `make grade`。实现一次通过，没有出现已证实需要修复的 AI 代码 bug。

| 评分项 | 分值 | 结果 | 主要依赖 |
| --- | ---: | --- | --- |
| Trap 入口、寄存器恢复、断点 EPC | 35 | PASS | 韩羽宸入口/恢复 + 林子媛断点处理 |
| 非法指令 EPC、软件/未知中断和兜底 | 25 | PASS | 林子媛异常处理 + 本人中断分类 |
| 时钟续设与重复 Trap 返回 | 40 | PASS | 本人时钟处理 + 韩羽宸返回路径 |
| 总计 | 100 | `100/100` | 三个模块联调 |

<a id="myx-p4"></a>

## MYX-P4：分析中断关闭与状态保存

**负责人：2410933 马禹翔**

报告原编号：**Prompt V2-A**。

### [PROMPT] 任务说明

**任务**：基于 `kern/driver/intr.c`、`kern/sync/sync.h`、`kern/mm/pmm.c` 和 `libs/atomic.h` 的真实代码，分析 Lab3 中断关闭机制、临界区入口和原子位操作的边界，并形成个人报告 4.4 的前半部分。

**操作要求**：只进行源码分析和报告写作，不修改内核实现。结论必须能由给定源码逐项验证，不把关闭中断夸大为多核锁或所有 Trap 的屏蔽机制。

**输出要求**：解释 `intr_disable()`、`__intr_save()`、`local_intr_save()`、物理内存管理临界区和 AMO 位操作；回答 `do { ... } while (0)` 思考题。

### [RELY] 上下文信息

```c
void intr_disable(void) {
    clear_csr(sstatus, SSTATUS_SIE);
}

static inline bool __intr_save(void) {
    if (read_csr(sstatus) & SSTATUS_SIE) {
        intr_disable();
        return 1;
    }
    return 0;
}

#define local_intr_save(x) \
    do {                   \
        x = __intr_save(); \
    } while (0)
```

`alloc_pages()`、`free_pages()` 和 `nr_free_pages()` 在访问 `pmm_manager` 前调用 `local_intr_save(intr_flag)`。`atomic.h` 使用 RISC-V AMO 指令实现置位、清位、翻转和 test-and-set/clear。

### [GUARANTEE] 交付清单

必须交付以下分析：

1. `intr_disable()` 与 `__intr_save()` 的状态变化和返回值含义；
2. 物理内存管理为何需要在更新空闲页结构时建立临界区；
3. `do { ... } while (0)` 的语法作用；
4. 关闭本 hart 中断与 AMO 原子位操作的联系、差异和适用边界。

不得修改任何 C 或汇编接口。

### [SPECIFICATION] 行为规格

#### 中断关闭分析

**Pre-Condition**:

- 分析对象是当前 hart 的 S 模式中断使能状态。
- 以源码中的 `sstatus.SIE` 和真实调用路径为依据。

**Post-Condition**:

- 清楚说明 SIE 原为 1 时，`__intr_save()` 关闭中断并返回 1；原为 0 时保持关闭并返回 0。
- 说明返回值保存的是进入临界区前的状态，而不是固定的成功标志。
- 说明内存分配器的空闲链表、空闲页计数和页属性需要作为一个整体维护，不能在中间状态被同 hart 的中断路径重入。
- 说明多语句宏包在 `do { ... } while (0)` 中后，可在 `if/else` 等语境中表现为单条语句，并要求调用者正常添加分号。

**Requirements**:

- 关闭 SIE 只屏蔽当前 hart 的 S 模式可屏蔽中断，不等于禁止异常，也不解决其他 hart 的并发访问。
- AMO 保证一次内存读改写不可分割；它适合单个位或单字操作，但不能自动保护由链表、计数和多个字段组成的复杂不变量。
- 长时间关闭中断会增加中断响应延迟，因此临界区应尽量短。

### 结果与后续调整

本任务只分析已有实现，没有修改内核代码。结论通过逐行核对 `intr.c`、`sync.h`、`pmm.c`、`atomic.h`，并推演初始开启、初始关闭和两层嵌套三种状态。没有发现源码冲突，也没有虚构运行时 bug。

<a id="myx-p5"></a>

## MYX-P5：分析中断恢复与条件开启

**负责人：2410933 马禹翔**

报告原编号：**Prompt V2-B**。

### [PROMPT] 任务说明

**任务**：继续分析 `__intr_restore()`、`local_intr_restore()` 和物理内存管理临界区出口，说明为什么恢复时必须依据保存的旧状态，而不能无条件开启中断，并形成个人报告 4.4 的后半部分。

**操作要求**：只进行分析和报告写作，不修改源码。用嵌套临界区和调用者原本已关闭中断的场景检查结论。

**输出要求**：解释条件恢复、嵌套语义、正常调用配对和机制边界，并与中断关闭分析组成完整章节。

### [RELY] 上下文信息

```c
void intr_enable(void) {
    set_csr(sstatus, SSTATUS_SIE);
}

static inline void __intr_restore(bool flag) {
    if (flag) {
        intr_enable();
    }
}

#define local_intr_restore(x) __intr_restore(x);
```

`alloc_pages()`、`free_pages()` 和 `nr_free_pages()` 在受保护操作结束后使用同一个 `intr_flag` 调用 `local_intr_restore(intr_flag)`。

### [GUARANTEE] 交付清单

必须交付以下分析：

1. `__intr_restore(true)` 和 `__intr_restore(false)` 的不同效果；
2. 为什么不能在临界区结束时无条件调用 `intr_enable()`；
3. 外层和内层临界区的保存、恢复顺序；
4. 该机制恢复的只是 SIE，而不是完整 `sstatus` 或跨 hart 的同步状态。

不得修改任何 C 或汇编接口。

### [SPECIFICATION] 行为规格

#### 中断恢复分析

**Pre-Condition**:

- `flag` 是同一临界区入口由 `__intr_save()` 返回的旧 SIE 状态。
- 受保护操作已经结束，控制流即将离开临界区。

**Post-Condition**:

- `flag` 为 1 时重新设置 SIE，恢复进入前的开启状态。
- `flag` 为 0 时保持中断关闭，尊重调用者或外层临界区原有状态。
- 嵌套情况下，内层在中断已关闭时保存 0，内层恢复不会提前开中断；最外层保存 1，最终由最外层恢复开启。

**Requirements**:

- 保存变量和恢复调用必须成对，且不能混用其他临界区的 flag。
- 受保护代码若增加提前返回或异常退出路径，必须保证相应恢复仍会执行。
- 该机制只恢复 `sstatus.SIE` 的逻辑状态，不是保存和还原所有 CSR 位。

### 结果与后续调整

本任务只分析已有实现，没有修改内核代码。结论通过逐行核对 `intr.c`、`sync.h`、`pmm.c`、`atomic.h`，并推演初始开启、初始关闭和两层嵌套三种状态。没有发现源码冲突，也没有虚构运行时 bug。

**2410936 林子媛：** 三组提示词摘自报告 4.3、4.5，分别对应完整实现规范稿、测试函数复现稿和 stvec 分析复现稿；结果与调试修正来自同一报告。

<a id="lzy-p1"></a>

## LZY-P1：生成异常处理函数

**负责人：2410936 林子媛**

按报告中已留存的完整四段式规范稿摘录；该稿是自然语言需求的规范整理，不是逐字聊天记录。报告中随后列出的简短实现复现稿与本组任务相同，不重复编号。

### [PROMPT] 任务说明

**任务：** 检查当前实验仓库和 Lab3 分工，确认林子媛（2410936）负责的是 `exception_handler(struct trapframe *tf)`。在不覆盖原 Lab2 工作的前提下完成本人异常函数，使非法指令、断点和其他异常分别满足既定行为。

**操作要求：** 先阅读 `code/kern/trap/trap.c`、`trap.h`、`trapentry.S` 及异常原因宏，确认真实接口再修改文件。只实现本人函数，保留既有 Trap frame、`SAVE_ALL`、分发和返回接口，不完成或署名组员的中断函数与恢复汇编。必要时保留组员函数的前向声明，但不能用空函数冒充实现。保留 `lab3`，把本人实现处的 `YOUR CODE` 替换为 `2410936`。

**输出要求：** 交付实际文件中的函数实现，解释异常原因、指令长度和返回地址的关系，给出后续测试方案。说明 `tf->epc` 与硬件 `sepc` 的区别，不以“能编译”代替运行验证，不提前宣称整组评分通过。

### [RELY] 上下文信息

工程为 RV64，目标包含 C 压缩指令扩展。与本任务直接相关的实际接口和常量为：

```c
#define CAUSE_ILLEGAL_INSTRUCTION 0x2
#define CAUSE_BREAKPOINT          0x3
#define CAUSE_FAULT_LOAD          0x5

struct trapframe {
    struct pushregs gpr;
    uintptr_t status;
    uintptr_t epc;
    uintptr_t badvaddr;
    uintptr_t cause;
};

void print_trapframe(struct trapframe *tf);
void exception_handler(struct trapframe *tf);
```

`struct pushregs` 保存 32 个通用寄存器槽位。已有 `SAVE_ALL` 将 `sstatus`、`sepc`、`stval`、`scause` 分别保存到帧中的 `status`、`epc`、`badvaddr`、`cause`。`trap_dispatch()` 用 `((intptr_t)tf->cause < 0)` 区分中断和同步异常，再调用相应处理函数。

这里只要求处理本实验中地址可读、长度为 16 或 32 位的内核故障指令。首半字低两位不为 `11` 时按 2 字节处理，为 `11` 时在本受测范围内按 4 字节处理。该简化不是针对任意 48 位以上编码的通用解码。

最终入口恢复由另一位成员负责：它应把更新后的 `tf->epc` 写回 `sepc` 后执行 `sret`。本人 C 函数不直接操作这个 CSR。

### [GUARANTEE] 交付清单

- 必须实现且保持签名：`void exception_handler(struct trapframe *tf)`。
- 在 `code/kern/trap/trap.c` 中保留 `//lab3 function2 2410936 (exception_handler)` 标记。
- 交付非法指令、断点和默认分支的处理，说明各分支允许改变的状态。
- 给出覆盖 16/32 位编码、返回位置及默认分支的测试设计。
- 不更改组员代码、原有评分脚本或评分条件，不实现用户地址安全取指、指令模拟或未要求的故障恢复。

### [SPECIFICATION] 行为规格

**Pre-Condition：** `tf` 指向有效 Trap frame，调用者已把中断分到其他路径。对非法指令和断点，EPC 指向可读、至少两字节对齐的受测指令起始位置。

**Post-Condition：** 非法指令与断点输出各自的规定提示，只将 `tf->epc` 前进到本指令之后；默认分支输出现场并保持整个帧不变。其余寄存器和 CSR 的实际恢复仍由统一汇编路径完成。

**Case 1——非法指令：** 原因码为 2，输出 `Illegal instruction at <原EPC>`，按首半字判断前进 2 或 4 字节。

**Case 2——断点：** 原因码为 3，输出 `Breakpoint at <原EPC>`。压缩断点和普通断点不能统一使用同一增量。

**Case 3——其他异常：** 只调用 `print_trapframe(tf)`，不读取 EPC 指向的指令，不修改 EPC，不声称程序一定能够恢复。

**Requirements：** 只改变允许的帧字段；输出地址必须是修改前的 EPC。保留现有签名、数据布局和注释要求。不可通过硬编码测试地址或统一增加固定偏移来代替长度判断。

### 结果与后续调整

Lab3 与原 Lab2 不属于直接沿最终提交历史继续的同一工作树，因此使用独立的 `Lab3` 目录，没有覆盖 Lab2。本人函数完成后，在已经覆盖的测试范围内一次通过，未观察到需要修改本函数的 bug；后续整合也保持该函数不变。共享功能二标记按实际负责人拆开，未把其他成员待完成位置换成本人学号。

<a id="lzy-p2"></a>

## LZY-P2：独立异常自测、真实指令探针与返回检查

**负责人：2410936 林子媛**

摘自报告的“测试函数复现提示词”，并将报告已有的测试上下文、交付要求及验收条件归入四段式。该提示词为事后复现稿，不是历史逐字输入；结果包含报告记录的整合版回归。

### [PROMPT] 任务说明

任务：为 exception_handler 设计独立、可重复的异常自测，不修改正式评分条件。

### [RELY] 上下文信息

四种真实编码为 `0x0000007b`、`0x0000`、`0x9002` 和 `0x00100073`，覆盖“非法指令/断点 × 16/32 位”。另用原因码 5、20 和 `EPC=0` 验证默认分支保持整个帧不变，是构造帧调用。

### [GUARANTEE] 交付清单

1. prepare.py 只向全新副本注入测试，拒绝覆盖已有目录，不修改提交源码。

2. 给出 GDB 三个检查点，不用 finish 猜测汇编返回位置；不得生成或伪造终端截图。

### [SPECIFICATION] 行为规格

1. 用四个显式编码覆盖 illegal32=0x0000007b、illegal16=0x0000、
   breakpoint16=0x9002、breakpoint32=0x00100073；预期 EPC 增量分别为 4、2、2、4。

2. 构造帧测试初始化所有字段，复制期望帧，仅调整期望 EPC，逐字节比较整个帧。

3. 汇编探针真实执行上述编码，预设临时、参数和保存寄存器；返回后比较，
   保留调用者的被调用者保存寄存器。32 位故障编码安排在只满足两字节对齐的位置。

4. 默认分支用 cause=5 和 cause=20、EPC=0，断言整个帧不变。
   明确这些是构造帧调用，不是实际访存异常恢复。

5. PASS 必须放在断言成功之后，失败时立即报错；输出 [LZY] 便于识别，
   但明确预期元数据、自测结果和仓库 make grade 的区别。

6. 队友入口未完成时，只在副本使用明确标示的参考入口和中断失败桩。
   队友完成后使用真实实现重新测试，并核对是否仍注入了参考代码。

### 结果与后续调整

本人异常函数的分类、提示、按长度更新 EPC、默认分支行为及实际返回均已验证；组员最终入口和中断实现合并后的普通运行、个人异常回归与仓库评分也已通过。

1. **预演使用 `finish` 误停。** Codex 自动预演时，`finish` 越过了当前异常返回，命中下一次 `exception_handler`。Trap frame 使用同一栈空间，读到下一次 EPC 后得出差值 424。这不是当前指令的正确长度，也不是已经证明的内核错误。改为在 `__trapret` 设置临时硬件断点，再在更新后的 EPC 设置断点，才把“C 已处理”和“CPU 已返回”分开观察；复测增量与返回位置均正确。
2. **手动输入错误与代码错误分开。** 我曾把 `$sepc` 写成带空格的表达式，也把 `$sp` 写成 `%sp`，GDB 报语法错误。修正命令后取得预期结果；这些命令没有证明内核出错，也没有成为修改内核的理由。图 6 保留了 `%sp` 输入错误及随后正确的 `$sp` 结果。

<a id="lzy-p3"></a>

## LZY-P3：stvec 模式编码的向后兼容

**负责人：2410936 林子媛**

摘自报告 4.5 的复现提示词、既有代码上下文和 SPEC 说明，按四段式重新排版；未单独保存当时的独立分析 prompt，不将本组标为历史原文。

### [PROMPT] 任务说明

请结合当前 idt_init() 和 __alltraps 的对齐设置，解释 stvec 新增 MODE 字段如何兼容旧用法。

### [RELY] 上下文信息

```c
extern void __alltraps(void);
write_csr(sscratch, 0);
write_csr(stvec, &__alltraps);
```

`__alltraps` 通过汇编 `.align(2)` 保证四字节对齐，其地址低两位为零，所以这里采用 Direct 模式，中断和异常都先进入 `__alltraps`，再由软件读取原因码分发。指令支持两字节对齐，不等于 `stvec` 的入口基址也可以只按两字节对齐。此外，向量槽位的步长 4 与本函数按故障指令长度增加的 2/4 字节是两件事，不能相互替代。

### [GUARANTEE] 交付清单

先列出旧地址四字节对齐产生的低位约束，再推导原样写入后 MODE 和 BASE 的值，
用一个具体地址验证；区分 Direct、Vectored 下同步异常和异步中断的入口。

### [SPECIFICATION] 行为规格

说明本实验实际使用哪种模式，不把向量表步长与故障指令长度混淆。
以规范编码和现有源码为依据，不声称本实验运行过未实现的向量模式。

本题需要说明“新增模式字段为什么不破坏旧的入口地址写法”，因此先确定旧用法的四字节对齐约束，再按新编码分离 BASE 和 MODE，最后与本实验的实际初始化代码对照。验收条件是入口地址和旧行为都保持不变，而不是只说“低位原来没用”。

### 结果与后续调整

本题没有记录到需要修复的代码 bug。核对重点是“向量模式并非把所有异常都分散到向量槽位”和“向后兼容不等于双向兼容”。本次采用编码推导与源码对照，没有修改 `stvec` 模式开展额外运行测试。
