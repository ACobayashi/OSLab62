# Lab2规范提示词与Answer——2410673韩羽宸

以下七组根据本人实际需求、工程定义和执行结果整理，不是逐字聊天记录。每组严格分为PROMPT、RELY、GUARANTEE、SPECIFICATION，后接真实Answer。操作约束描述对应实施阶段；现按用户授权合入正式报告。正文对应位置见 [个人报告](2410673_韩羽宸.md)。本文件仅含韩羽宸部分，队友的Buddy/SLUB两份提示词由各自交付。

实现任务强调修改真实文件，内部C辅助函数使用static且不改变公共接口；分析和只读调试任务交付实际材料，不为套用实现模板而改内核。所有补充测试和调试助手均只在附加材料运行，正式code保持实验源码。

## P1 物理内存探测与页初始化

### [PROMPT]任务说明

**任务：** 在当前 Lab2 项目的 `code/kern/mm/pmm.c` 中，实现 `static void page_init(void)`，将设备树提供的物理内存信息转换为页描述符布局和当前物理内存管理器的初始空闲区。需要正确区分内核虚拟地址、物理地址、物理页号与 `struct Page *`，使 OpenSBI、内核镜像、页描述符数组及对齐空隙占用的页面不会被当作空闲页。

**操作要求：** 必须修改项目中的实际文件，只在 `page_init()` 待补全位置实现本任务，并将原有 `lab2 function1 YOUR CODE` 注释中的 `YOUR CODE` 替换为 `2410673`。保留其他现有代码，不改动接口、宏、数据结构、设备树解析器、已有测试、链接脚本和启动汇编。本阶段保持当前管理器选择，不提前实现或切换 Best Fit。不要修改队友的 Buddy 模块。验证需要的临时程序、日志及构建产物全部放在“附加材料/阶段2_物理内存初始化”下，不向 `code/` 添加辅助文件。

**输出要求：** 交付完整函数实现、布局计算说明以及实际验证结果。输出探测范围、实际管理范围和完整空闲页区间，说明空闲页数。验证既要覆盖正常路径，也要覆盖不按页对齐、内存不足和无效设备树结果等分支。工程中的真实定义优先于任何示例；独立测试通过与完整内核通过必须分别记录。

### [RELY] 上下文信息

当前工程使用 RV64，以下定义来自实际代码。

`libs/defs.h` 的基础类型：

```c
typedef unsigned long long uint64_t;
typedef uint64_t uintptr_t;
typedef uintptr_t size_t;
```

`libs/riscv.h`、`kern/mm/mmu.h`、`kern/mm/memlayout.h` 中与本任务直接相关的值：

```c
#define DRAM_BASE 0x80000000
#define PGSIZE 4096
#define PGSHIFT 12
#define PPN(la) (((uintptr_t)(la)) >> PGSHIFT)
#define PHYSICAL_MEMORY_END 0x88000000
#define PHYSICAL_MEMORY_OFFSET 0xFFFFFFFF40000000
#define KERNEL_BEGIN_PADDR 0x80200000
#define KERNBASE 0xFFFFFFFFC0200000
```

`Page` 与链表节点的实际字段：

```c
struct list_entry {
    struct list_entry *prev, *next;
};
typedef struct list_entry list_entry_t;
struct Page {
    int ref;
    uint64_t flags;
    unsigned int property;
    list_entry_t page_link;
};
```

当前 RV64 布局下 `sizeof(struct Page)` 为 40 字节，代码必须使用 `sizeof`，不能把该数值写死。

`pmm.c` 已有的状态和委托接口：

```c
struct Page *pages;
size_t npage = 0;
uint64_t va_pa_offset;
const size_t nbase = DRAM_BASE / PGSIZE;
const struct pmm_manager *pmm_manager;

static void init_memmap(struct Page *base, size_t n) {
    pmm_manager->init_memmap(base, n);
}
```

可依赖的已有函数及链接符号：

```c
uint64_t get_memory_base(void);
uint64_t get_memory_size(void);
static inline void set_page_ref(struct Page *page, int val);
static inline struct Page *pa2page(uintptr_t pa);
extern char end[];
```

这些只是已有接口的上下文清单，不要重复添加定义。`end` 是 `tools/kernel.ld` 提供的内核末尾虚拟地址；`pages` 也保存虚拟地址。`page2pa(p)` 对应物理页号 `p - pages + nbase`，`pa2page(pa)` 对应 `&pages[PPN(pa) - nbase]`，因此描述符总数是 `npage - nbase`。`npage` 是绝对物理页号的上界。

使用实际 `PADDR(kva)` 进行虚实地址转换：它会检查地址不低于 `KERNBASE`，并减去 `va_pa_offset`。在它被使用时，偏移必须已正确设置。对齐使用 `defs.h` 的 `ROUNDUP` / `ROUNDDOWN`，页面状态使用 `SetPageReserved` / `PageReserved` / `PageProperty` 等实际宏，链表节点使用 `list_init()`。`panic()` 终止执行，`cprintf()` 输出验证所需信息。

本项目 `nbase` 固定，支持 QEMU virt 上从 `DRAM_BASE` 开始的一段 RAM，不要求支持任意物理内存起点或多段 RAM。设备树默认报告 128 MiB。按手册简化约定，DTB 中的起点与长度提取后，其存储可以复用。

### [GUARANTEE] 接口清单

- 必须实现且保持签名：`static void page_init(void)`，位于 `code/kern/mm/pmm.c`。
- 成功时对外提供一致的 `va_pa_offset`、`npage`、`pages` 和当前管理器的空闲页初始状态。
- 沿用 `init_memmap(struct Page *base, size_t n)` 委托接口，不新增或修改公开接口。
- 不需要增加辅助函数；如确有必要，辅助函数只能为模块内部的 `static` 函数，并且不能扩大允许修改的范围。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 启动分页和内核 BSS 初始化已经完成，设备树解析已执行。当前物理内存管理器已执行自身 `init()`，尚未登记本轮空闲区；`end` 可转换为实际内核末尾物理地址。该函数在启动阶段调用一次，不是可重复重置运行中分配器的接口。

**Post-Condition：** 正常返回时，虚实偏移等于 `PHYSICAL_MEMORY_OFFSET`。可管理物理地址上界不超过设备树所述末端和 `PHYSICAL_MEMORY_END`，并且按页向下对齐。`npage` 表示该上界的物理页号，描述符连续覆盖 `[nbase, npage)`。数组首地址位于 `end` 之后并按页对齐，数组本身完整落在可管理 RAM 内。

所有描述符在交给管理器前应具有确定的初始值：引用计数为零、`property` 为零、只有保留标志置位，链表节点自环。空闲区只包含描述符数组物理末端向上按页对齐后、管理上界之前的完整页面。该区之外的页保持保留，不能被分配。正常打印应能够核对原始范围、实际管理范围和空闲页数。

**Case 1：正常 RAM。** 默认 128 MiB 时，管理区为 `[0x80000000, 0x88000000)`，描述符共 32768 个；空闲页数由本次实际内核大小和描述符占用决定，不写死。若有完整空闲页，仅用对应的描述符起点和准确页数登记一次。

**Case 2：RAM 大小变化或末端不对齐。** 小于管理上限时按照实际大小计算；大于管理上限时只管理上限之内的页面；内存末端不足一页的部分不加入分配器。原始探测结果与实际管理范围应分别可见。

**Case 3：无完整空闲页或恰好一页。** 在描述符数组本身能够容纳的前提下，零空闲页时可以完成描述符初始化，但不得调用 `init_memmap()`，空闲计数保持零；恰好一页时只登记这一页。

**Case 4：输入无效。** RAM 起点不符合工程固定布局、大小为零，或起点加长度会发生无符号溢出时，使用 `panic()` 终止，不能向分配器登记页面。

**Case 5：空间不足。** 管理范围不能覆盖内核，或不能容纳整个描述符数组时，使用 `panic()` 终止。尤其要在写入描述符前确认整个数组放得下，不能依靠写越界后出现的异常发现错误。

**Requirements：** 不修改已有测试来获得通过，不跳过保留区，不把物理地址直接当作描述符指针。独立测试需检查所有描述符的状态和保留边界，并验证分配整个空闲区、耗尽后失败、释放后空闲页数恢复；完整工程测试待 Best Fit 模块完成后再执行和记录。

### Answer：实际交付与迭代

在实际 `pmm.c` 中补全 `page_init()`，标记 `2410673`。实现了 DTB 结果校验、管理上限、描述符布局、保留区及完整空闲页登记。第一次完整执行 13 个初始化用例均通过，未观察到需要修正的初始化算法错误。初次测试驱动因调用只有声明的 `sbi_shutdown()` 链接失败，仅在附加材料中改为局部退出调用；没有修改正式 SBI 模块。后续完整内核中任务一 PASS，详见个人报告 4.2、5.1、5.2。

## P2 Best Fit 完整模块与接入

### [PROMPT]任务说明

**任务：** 在当前 Lab2 工程 `code/kern/mm/best_fit_pmm.c` 的待补全位置，实现完整的 Best Fit 连续物理页分配模块，定义 `best_fit_pmm_manager`，并在 `code/kern/mm/pmm.c` 的 `init_pmm_manager()` 中选择该管理器。对于一次申请，应返回所有足够大的空闲连续块中页数最少的块；若多个候选大小相同，选择物理地址较低者。支持精确匹配、较大块拆分、释放后的相邻块合并和空闲页计数。

**操作要求：** 必须修改工程中的实际文件，保留 `lab2 function2-2` 注释，将 `YOUR CODE` 替换为 `2410673`。不得修改文件中已有的 `basic_check()`、`best_fit_check()`，也不得改动已有头文件包含、`pmm.h`、`memlayout.h`、`Page` 结构、宏或公开接口。保留阶段二的 `page_init()`，不改动队友的 Buddy 模块。本任务实现范围是 Best Fit 模块及管理器选择；如完整启动发现其他问题，先提供实际诊断证据，再单独处理。

验证程序、评分副本、日志和构建产物全部放入“附加材料/阶段3_BestFit”，不向正式 `code/` 目录加入辅助程序或测试注入文件。正式代码需要通过真实 RV64 编译、原有检查和适用评分，不能通过改动测试断言获得通过。

**输出要求：** 交付完整代码、规范的行为说明、实际验证记录和改进空间分析。说明链表的组织依据、Best Fit 的选择依据、拆分后剩余区如何保留、释放后为何能够合并，以及失败时状态是否保持不变。区分原有测试、补充测试和最终评分各自证明的内容。

### [RELY] 上下文信息

以下内容来自当前工程，不要根据其他版本猜测名称或布局。

`code/kern/mm/pmm.h` 定义的管理器接口为：

```c
struct pmm_manager {
    const char *name;
    void (*init)(void);
    void (*init_memmap)(struct Page *base, size_t n);
    struct Page *(*alloc_pages)(size_t n);
    void (*free_pages)(struct Page *base, size_t n);
    size_t (*nr_free_pages)(void);
    void (*check)(void);
};
```

`code/kern/mm/memlayout.h` 中的管理数据结构为：

```c
struct Page {
    int ref;
    uint64_t flags;
    unsigned int property;
    list_entry_t page_link;
};
typedef struct {
    list_entry_t free_list;
    unsigned int nr_free;
} free_area_t;
```

`PG_reserved` 为保留标志，`PG_property` 表示空闲块头；`property` 记录该块页数，只有空闲块头的该字段有意义。使用已有的 `SetPageProperty`、`ClearPageProperty`、`PageReserved`、`PageProperty`、`set_page_ref()`，以及 `list_init()`、`list_next()`、`list_prev()`、`list_add_before()`、`list_add_after()`、`list_del_init()` 和 `le2page()`。`list_entry_t` 包含 `prev` / `next` 两个指针，链表头是循环双向链表的哨兵节点。

`pmm.c` 已提供 `pages`、`npage`、`nbase` 和 `pmm_manager`。描述符范围为 `[pages, pages + (npage - nbase))`，描述符顺序对应物理页地址顺序。`alloc_pages()`、`free_pages()`、`nr_free_pages()` 分别委托给当前管理器的函数指针。`page_init()` 已经建立描述符，并把可用区先标为保留后交给管理器的 `init_memmap()`。

`code/kern/mm/best_fit_pmm.h` 已声明：

```c
extern const struct pmm_manager best_fit_pmm_manager;
```

该文件原有检查函数直接访问 `free_list` 和 `nr_free`，并在部分测试过程中临时保存、置空和恢复这两项状态。因此实现必须提供兼容的内部名字，允许“总数暂未重置，但链表已经置空”的测试中间状态，并保证找不到连续块时返回 `NULL`。`best_fit_check()` 内部会调用 `basic_check()`，注册后的 `.check` 必须使用原有检查函数。

`code/kern/mm/default_pmm.c` 是框架已提供的 First Fit，可阅读其状态含义和接口语义。Best Fit 的候选选择必须满足本任务规格，不能只选择遇到的第一个足够大的块。当前工程物理内存管理上界为 `0x88000000`，从 `0x80000000` 开始，最多管理 32768 个页描述符。

### [GUARANTEE] 接口清单

必须交付下列管理函数，并通过注册结构连接到已有公共接口：

```c
static void best_fit_init(void);
static void best_fit_init_memmap(struct Page *base, size_t n);
static struct Page *best_fit_alloc_pages(size_t n);
static void best_fit_free_pages(struct Page *base, size_t n);
static size_t best_fit_nr_free_pages(void);
const struct pmm_manager best_fit_pmm_manager;
```

原有 `static void best_fit_check(void)` 保持函数体原样，用作 `.check` 回调；原有 `basic_check()` 同样保持不变。允许为插入和合并等内部操作添加 `static` 辅助函数，不增加公开 API。`init_pmm_manager()` 选择 `&best_fit_pmm_manager`。

### [SPECIFICATION] 行为规格

**整体不变量：** 空闲链表按物理地址递增，节点只代表空闲块头，块间不重叠，物理相邻的空闲块应已合并。每个块头 `PG_property` 置位、`property > 0`；非块头不带该标志。空闲页总数等于各块长度之和，不等于块的数量。所有分配、释放操作按当前单核启动实验的顺序调用，本任务不实现并发锁。

**`best_fit_init()`**

- Pre-Condition：管理器尚未登记空闲区；该接口用于初始化，而不是运行中丢弃已有分配状态。
- Post-Condition：循环链表为空，空闲页总数为零。

**`best_fit_init_memmap(base, n)`**

- Pre-Condition：`n > 0`，连续描述符完整位于管理范围内，对应页面均处于保留状态，且没有与已经登记的空闲区重叠。
- Post-Condition：这 `n` 页不再保留，引用计数清零；空闲区按地址组织，块头元数据正确，空闲页数恰好增加 `n`。允许先后登记多个区域，登记顺序可以不同于地址顺序。
- Case：新区域与已有空闲区物理相邻时合并，不相邻时保持独立；不能越过保留区合并。

**`best_fit_alloc_pages(n)`**

- Pre-Condition：`n > 0`；零页请求违反接口约定，应触发断言。
- Post-Condition：成功时返回最小合适块的起始描述符，分配页数恰为 `n`，总空闲页数减少 `n`，已分配区不再是空闲块头。
- Case 1：候选块恰有 `n` 页，整块交付，不产生零长度空闲块。
- Case 2：候选块大于 `n` 页，交付前端 `n` 页，剩余尾部保留为空闲块，保持地址顺序。
- Case 3：存在多个等大的最佳候选，选择地址最低者，保证结果可复现。
- Case 4：总空闲页不足，或总数足够但没有足够长的连续块，返回 `NULL`，链表、块长度和总计数不变。临时空链表也不得产生无效指针。

**`best_fit_free_pages(base, n)`**

- Pre-Condition：`n > 0`，区间在管理范围内，所有页面当前已分配，未保留、未重复释放。调用者须提供准确区间；本模块不额外维护分配历史来验证任意非法释放。
- Post-Condition：区间重新可分配，引用计数和内部块长度清理，空闲页总数恰好增加 `n`。仅按真实物理相邻关系合并，不因为两个块在链表中相邻就无条件合并。
- Case：分别支持无邻居、仅前块相邻、仅后块相邻、同时连接前后两块。被合并掉的旧块头不得残留有效 `PG_property` 标记。

**`best_fit_nr_free_pages()`**

- Pre-Condition：管理器状态有效。
- Post-Condition：返回当前空闲页总数，不改变任何状态。

**验证要求：** 原有两项检查函数保持原样并实际运行；完整内核应显示 Best Fit 管理器名称及检查通过。补充验证最小合适选择、精确匹配、同尺寸选择、拆分、碎片不足、不同释放顺序和两侧合并。使用独立页面占用模型逐步核对一组重复分配与释放操作，检查指针、计数和不重叠性质，不能只凭“没有崩溃”判断正确。

### Answer：实际交付与迭代

完整 Best Fit 模块及管理器切换已实现，保留原有 `basic_check()`、`best_fit_check()`。原有检查得到 6/6，任务二 PASS，适用评分 20/20；补充 10 组行为验证及 3000 次独立模型操作通过。算法查找、拆分、左右合并和计数均有实际验证，没有虚构额外算法 bug。首次集成失败来自原启动分页过渡，另以 P3 处理。详见个人报告 4.5 和 5.2。

## P3 启动分页过渡修复

### [PROMPT]任务说明

**任务：** 修复实际 `code/kern/init/entry.S` 中从 Bare 切换到 Sv39 后无法继续取指的问题，使完整 Lab2 内核能够进入 `kern_init()` 并执行内存管理检查。

**操作要求：** 修改实际启动汇编，标明 2410673。保留现有高地址链接、虚实偏移、根页表、OpenSBI 参数保存和原有 C 入口，不改动分配器或已有测试。只增加启动过渡所需的最小映射和控制流，进入高地址后必须移除临时映射并刷新相关地址翻译状态。

**输出要求：** 解释原故障的实际证据、临时映射覆盖的区间、何时跳转和何时清理。运行完整内核并用 GDB 核对最终页表，不得只根据输出猜测修复有效。

### [RELY] 上下文信息

原始完整内核已成功编译，但没有输出 C 初始化信息。GDB 单步的实际结果为：`satp = 0x8000000000080205`、`scause = 0xc`、`stval = 0x80200038`。对应位置为写入 `satp` 后的 `sfence.vma` 指令，说明低地址取指产生了 instruction page fault。

原始代码以 `0xffffffffc0200000` 为链接起点，物理加载地址为 `0x80200000`，偏移为 `0xffffffff40000000`。根页表包含 512 个 8 字节项，原来仅第 511 项有效，其值为 `(0x80000 << 10) | 0xcf`，将高地址区域映射到从 `0x80000000` 起的 1 GiB 物理区。开启分页不会自动改变当前 PC 的数值。

根页表第 2 项对应从 `0x80000000` 起的低地址 1 GiB 区域。若临时填入同样的大页叶项，则启动代码当前 PC 所处的物理区可以继续按同值虚拟地址取指。实际根表符号为 `boot_page_table_sv39`，C 入口为 `kern_init`，栈顶为 `bootstacktop`。

### [GUARANTEE] 接口清单

- 保持入口符号 `kern_entry`、根表符号 `boot_page_table_sv39`、`boot_hartid`、`boot_dtb`、`bootstack` 和 `bootstacktop` 的职责。
- 保持页表为一页、512 项，不改变最终高地址映射和原有 C 接口。
- 可增加仅供汇编内部使用的跳转标签，例如 `.Lhigh_entry`，不新增公共函数。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 内核在 `0x80200000` 开始执行，处于启动阶段，尚未使用 C 栈，OpenSBI 的 hartid 与 DTB 参数仍可保存。页表尚未作为运行中共享结构使用。

**Post-Condition：** 控制流安全进入高地址，再使用高地址栈和 C 入口。进入 C 初始化后，第 2 个根页表项为零，第 511 项仍为 `0x200000cf`，`satp` 处于 Sv39 且仍指向原根表。

**Case 1：切换时低地址 PC。** 开启 Sv39 后，到高地址跳转之前的指令必须有有效取指映射，不能依赖未设置的异常处理程序进行恢复。

**Case 2：已经进入高地址。** 临时低地址映射应被清除，之后执行 `sfence.vma`；清除动作和后续指令须依靠高地址映射继续执行。

**Requirements：** 保留失败启动日志和单步寄存器证据；修复后既要通过完整内核检查，也要用 GDB 观察实际 PC、`satp` 和两个相关根页表项。

### Answer：实际交付与迭代

真实故障为 `scause=0xc`、`stval=0x80200038`：写入 satp 后 PC 仍低，而原页表只映射高地址。增加根表第 2 项的临时恒等映射，跳到高地址后清零该项并执行 sfence.vma，再设置栈与进入 C。修复后完整内核和原有评分通过；现场启动观察确认进入 C 后 root[2]=0、root[511]=0x200000cf。详见个人报告 4.1 及启动截图。

## P4 原有检查与补充验证

### [PROMPT]任务说明

**任务：** 为已经实现的 `page_init()` 和 Best Fit 连续物理页分配模块建立可复现的验证方案。先运行工程原有检查和评分，再根据手册行为要求补充初始化边界、最小合适块选择、拆分、合并及计数检查。输出实际结果和后续报告可引用的证据。

**操作要求：** 使用真实工程文件与接口，禁止修改原有 `basic_check()`、`best_fit_check()`、`tools/test_lab2.c`、`tools/inject.py` 和评分断言来获得通过。不得向正式 `code/` 添加辅助脚本、测试注入文件或构建产物，不修改队友模块。新增测试驱动、执行脚本、构建副本与日志必须实际保存在附加材料中；引用正式实现，不能复制另一份分配算法代替被测代码。

完整工程检查和隔离测试分别记录。注入测试只在副本执行，保留正式初始化逻辑和 Best Fit 管理器选择。用户现场运行并自行截图，助手仅核对、命名和归档原图，不绘制终端截图，不用预设 PASS 代替真实执行。

**输出要求：** 说明每类测试的初始条件、操作、预期和实际结果，明确原有检查与新增验证的来源。保存原始日志和被测源码哈希，报告优先展示与手册直接相关的完整内核运行和原有评分图。出现错误时保留失败证据，修复真实原因后再验收，不扩充无关测试或重复配图。

### [RELY] 上下文信息

工程的被测入口与公共接口为：

```c
static void page_init(void);  // pmm.c 内部函数
void pmm_init(void);
struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);
```

`page_init()` 根据 `get_memory_base()`、`get_memory_size()` 设置 `va_pa_offset`、`npage` 和 `pages`，再通过当前管理器登记空闲区。`nbase = DRAM_BASE / PGSIZE`，描述符元素数为 `npage - nbase`。当前框架 `PGSIZE = 4096`、`DRAM_BASE = 0x80000000`、`PHYSICAL_MEMORY_END = 0x88000000`，采用从固定起点开始的单段 RAM 布局。

```c
struct Page {
    int ref;
    uint64_t flags;
    unsigned int property;
    list_entry_t page_link;
};
```

空闲块头带 `PG_property` 标志，`property` 表示连续块页数；保留页带 `PG_reserved`，不得被登记或分配。Best Fit 链表按地址排序，候选选择为最小的足够大块，同尺寸时选择低地址者；正页数请求是接口前提，连续空间不足返回 `NULL`。

`best_fit_pmm_manager.check` 指向原有 `best_fit_check()`，它会调用原有 `basic_check()`。编译时使用 `ucore_test` 可以显示六项检查进度。工程自带的 `test_page_init()` 检查正常初始化，`test_best_fit()` 检查当前管理器接入，`make grade` 在副本中注入检查并对这两项评分。当前内核完成初始化后进入无限循环。

当前环境为 WSL Ubuntu-22.04、RISC-V GCC 10.2、QEMU 7.0.0 和 OpenSBI 1.0。启动命令需要向 QEMU 显式传入 `-kernel bin/ucore.img`，同时保留原有 loader 参数。各工具路径和原始命令以实际执行记录为准。

### [GUARANTEE] 交付清单

- 初始化隔离验证：`阶段2_物理内存初始化/run_tests.py` 及其测试驱动，直接调用实际 `page_init()`，输出每项结果和 `test_output/summary.json`。
- Best Fit 隔离验证：`阶段3_BestFit/run_boundaries.py` 与 `best_fit_boundary_test.c`，直接测试正式实现，保存行为测试、独立模型核对结果和源码哈希。
- 完整内核验证：`阶段3_BestFit/run_kernel.py`，复制源码、调用原有注入工具、构建并观察原有检查和两项任务检查。
- 原有评分验证：`阶段3_BestFit/verify_grade.py`，实际执行原有 `make grade` 并保存 `verification/official_grade.log`。
- 报告交付：测试报告草稿、原图与用途记录、测试来源说明、规范提示词和基于真实结果的答复。归档图片使用 `report/images/hyc/`，不修改正式报告正文的既定结构。

以上为实际脚本与输出的交付清单，不要求增加任何公开 C 接口。隔离驱动中的包装和辅助函数只服务测试，不进入正式提交代码。

### [SPECIFICATION] 行为规格

**初始化验证。**

- Pre-Condition：使用真实页描述符定义、地址宏和初始化实现，管理器先处于空状态；正常场景使用真实 DTB，注入场景清楚标明输入来源。
- Post-Condition：成功时描述符位置、数量、保留状态与空闲计数准确；空闲区不会包含内核或描述符占用页，不完整尾页不登记。
- Case：覆盖默认 RAM、较小 RAM、超过管理上限、末端不对齐、零或一页可用、无效起点或大小、范围溢出、不能覆盖内核和不能容纳描述符。
- 异常用例只有在捕获预期终止且未登记空闲页时才通过；不能把异常打印本身当作正常初始化成功。零空闲页时检查 `init_memmap()` 调用次数为零。

**Best Fit 验证。**

- Pre-Condition：在有效描述符池中建立可说明的保留、空闲及已分配状态，请求和释放遵循实际接口前提。
- Post-Condition：返回位置与最小合适块策略一致，拆分与合并不产生重叠或计数错误，失败申请保持原状态。
- Case：覆盖空表、乱序登记、多个候选、精确匹配、同尺寸选择、拆分、碎片导致的不足、总量不足、左右邻居合并和耗尽恢复。
- 另以独立逐页占用模型推导预期候选，逐次检查申请与释放；模型不读分配器链表作为预期答案。最后归还全部存活申请并核对恢复。

**完整运行与评分。**

- Pre-Condition：副本来自当前正式源码，原有测试保持不变，使用与本机固件兼容的启动参数。
- Post-Condition：实际运行选择 Best Fit，原有分配检查和两项任务检查通过，评分输出可核对。源码哈希、日志、用户截图应能对应同一实现版本。
- Case：启动失败、panic、检查缺失或分数不足都不得标记为通过。内核本来会无限循环，应通过预期输出判断初始化是否完成，不能只根据 QEMU 未自行退出判断失败。
- Requirements：评分的 20/20 仅代表现有评分项，不代表 Buddy、SLUB、理论题或双 GDB 调试全部完成；隔离测试通过也不能代替完整工程检查。

### Answer：实际交付与迭代

原有检查、两项任务检查及评分保持不变。初始化 13 个用例通过；Best Fit 10 组检查、3000 次模型操作及累计 3060 次状态核对通过；原有评分 20/20。用户现场截图已归档，正文采用完整运行和评分图，补充测试图以链接保留。异常初始化用例的 PASS 表示捕获预期终止，不是无效输入初始化成功；评分不覆盖 Buddy、SLUB 或全部调试。详见个人报告第五章。

## P5 启动原理、六问与物理内存 Challenge

### [PROMPT]任务说明

**任务：** 通读本地 Lab2 手册中与韩羽宸职责对应的启动分页、物理初始化和思考题内容，结合当前 `kernel.ld`、`entry.S`、`init.c`、`dtb.c`、`pmm.c` 及 Best Fit 实现，交付可用于报告和学习的正文材料。逐条回答分页设计六问，完成物理内存范围获取 Challenge，并说明本实现 Best Fit 的改进空间与代价。

**操作要求：** 必须读取实际文件，以当前代码为准区分手册示意和真实执行。不要修改 `code/`、正式报告正文或队友模块。所有新增 Markdown、观察脚本和日志实际写入“附加材料/阶段5_启动分页与思考题”。使用四段式整理提示词，不虚构历史交互、代码错误、运行截图或尚未完成的双 GDB 实验。

为说明启动过程，可在已有且与当前源码对应的测试内核上使用 RISC-V GDB 读取寄存器、符号和根表，允许设置断点、继续和单步；不得通过手工设置 PC、`satp`、PTE 或跳过故障来制造正常结果。观察结束后清理本脚本启动的 QEMU，不结束用户其他进程。用户自行执行命令和截图，助手提供步骤并归档。

**输出要求：** 正文沿用已有编号，详细解释放在段落内，不增加大量小标题。给出六问答案、初始化主线、真实启动观察、DTB 交付和解析路径、Challenge 的方法与限制、Best Fit 改进分析、答辩问答和复现步骤。外部体系结构与固件信息使用官方规范核对，理论方法与本实验实际实现分别说明。

### [RELY] 上下文信息

本地手册为 `E:/Grade3_1/操作系统/lab_book/手册完整版.html`，重点页面是练习、基本原理、分页设计六问、实现分页机制、物理内存管理、物理内存探测及报告要求。

当前工程的相关常量：

```c
#define DRAM_BASE 0x80000000
#define PGSIZE 4096
#define PGSHIFT 12
#define KERNBASE 0xFFFFFFFFC0200000
#define PHYSICAL_MEMORY_OFFSET 0xFFFFFFFF40000000
#define PHYSICAL_MEMORY_END 0x88000000
#define KSTACKPAGE 2
#define KSTACKSIZE (KSTACKPAGE * PGSIZE)
```

`kernel.ld` 的 `BASE_ADDRESS` 为 `0xFFFFFFFFC0200000`，入口符号为 `kern_entry`，并提供 `etext`、`edata`、`end`。`entry.S` 在 `.data` 静态放置启动栈和按页对齐的 512 项根表，根项值为 `(0x80000 << 10) | 0xcf`；第 511 项为高地址叶映射，第 2 项为本次修复增加的过渡期恒等映射，高地址标签中将其清零。

汇编先保存 `a0/a1` 到 `.data` 的 `boot_hartid/boot_dtb`，将根表物理页号与 `8 << 60` 组合后写入 `satp`，同步地址翻译并切换 PC，清理临时根项，设置 `bootstacktop` 后跳转 `kern_init`。当前 `kern_init()` 顺序为 BSS 清零、`dtb_init()`、`cons_init()`、信息输出、`pmm_init()`。

DTB 解析器把启动参数中的物理地址加上编译期映射偏移，检查魔数，读取 memory 节点的 `reg`，保存 `memory_base/memory_size`，再通过 getter 提供给 `page_init()`。当前实现只处理启动时固定布局的一段 RAM；不能把它描述成完整通用的设备树内存发现库。

Best Fit 已接入当前管理器，按地址链表组织空闲块，分配时扫描最小合适候选，释放时按物理邻接合并。原有检查和评分已经通过，阶段四已取得用户现场截图；阶段五不需要重新制作测试汇总图。

可以复用的已测内核位于 `附加材料/阶段3_BestFit/kernel_tree/bin/`，对应源码哈希在阶段三日志中。已有 RISC-V GDB 为 `/home/smile/toolchains/riscv-elf-toolchains/bin/riscv64-unknown-elf-gdb`，运行环境为 WSL Ubuntu-22.04、QEMU 7.0.0。开始观察前先确认当前正式源码仍与该已测版本一致。

### [GUARANTEE] 交付清单

- `启动分页与思考题报告草稿.md`：对应既有 3.1～3.3、4.1、4.2 补充、4.3、4.5 思考题内容。
- `阶段5学习与验收指引.md`：手册对照、学习顺序、答辩要点和用户截图步骤。
- `observe_boot.py`：实际入口为 `main()`，只读观测现有内核，生成 GDB 命令、日志、根表快照和结构化结果。不新增任何内核公开接口。
- `观察记录/gdb_boot_trace.log`、`boot_observations.json`、`final_root_page_table.bin`：本轮真实观察证据。
- 本文件：四段式规范提示词、实际答复和本轮问题记录；后续收录正式提示词文件。

### [SPECIFICATION] 行为规格

**报告与六问回答。**

- Pre-Condition：已经通读相关手册小节及当前源码，明确第三阶段修改过启动过渡代码。
- Post-Condition：六问逐条对应实际物理映射范围、映射关系、根表地址、表空间、目录项和叶项内容，能从源码或真实结果追溯。区分一般 Sv39 三层结构与当前根级 1 GiB 叶项，区分映射窗口、真实 RAM 和可分配区。
- Case：比较原始高地址单项根表、当前启动过渡期两项有效及进入 C 后只剩高地址项三个状态；不能说所有时刻前 511 项都为空。符号地址和伪指令实际结果应根据链接及反汇编解释。

**Challenge 与改进题。**

- Pre-Condition：知道本实验使用 DTB，并且没有实际调用 BIOS/UEFI 的探测程序。
- Post-Condition：说明启动信息协议、RAM 区间与用途、保留区排除及完整页组织。至少联系 DTB、引导程序交付和其他固件地址图方法，交代平台依赖和当前单段解析的限制。
- Case：不能把未知地址连续写测试值当作通用安全探测；不能仅凭固件接口的头文件声明断言功能可用。Best Fit 的改进需讨论查找开销、索引维护和碎片取舍，未实现的优化不得宣称已测加速。

**`observe_boot.py` 的观察行为。**

- Pre-Condition：现有内核镜像和 ELF 存在且源文件哈希对应当前代码，调试器及 QEMU 可用，输出目录位于附加材料。
- Post-Condition：采集 Bare 入口、写 `satp` 前、开启 Sv39 后但 PC 仍低、到达高地址但尚未清理、C 入口、DTB 已解析而物理管理即将开始等六个真实时刻；每项记录 PC、SP、`satp` 和相关根项。
- Case：开启分页后 PC 仍低时，第 2 根项有效；进入 C 后该项为零，第 511 项保留，栈为内核的 8 KiB 栈。保存的 hartid/DTB 与入口参数一致，DTB 的内存结果在物理管理前已经可读。
- Requirements：如缺少断点、符号或导出数据，应明确报错并保留日志，不能填入预期数值。脚本只读客体状态，不手动设置客体寄存器或页表。原始日志与结构化摘要同时保存，最终清理自己启动的进程。

### Answer：实际交付与迭代

结合真实代码完成分页设计六问、启动与 DTB 主线、物理内存范围 Challenge 及 Best Fit 改进分析。GDB 记录了启动六个时刻，用户提供现场图。Challenge 分析 DTB、固件/引导程序内存图、保留区和对齐；本工程未实现 E820/UEFI 探测。观察脚本初次因 dump binary memory 的文件名引号导出失败，仅修改附加材料的路径格式后重跑成功。详见个人报告 3、4.1、4.3。

## P6 双 GDB 调试环境

### [PROMPT]任务说明

**任务：** 阅读实验手册“双重 GDB 调试 QEMU 地址翻译过程”小节，结合现有 WSL Ubuntu-22.04 环境，建立宿主 GDB 调试 QEMU、RISC-V GDB 调试 ucore 的联动环境。先验证两个调试器的目标、源码符号、连接和暂停顺序，再提供三个终端的分步操作，让用户自行操作并截取与手册直接相关的画面。

**操作要求：** 读取实际工程、工具和构建信息。优先复用匹配源码且具有调试信息的 QEMU；若已有构建满足要求，不为复现手册版本而覆盖日常安装。不修改系统附加权限，不要求保存密码。允许宿主 GDB 启动本次 QEMU 子进程，客体 GDB 经本机回环地址连接 stub。仅控制自己启动的进程，不终止其他 QEMU 会话。

必须把脚本、配置、真实日志及报告素材写入 `附加材料/阶段6_双GDB调试环境`；本阶段不修改 `code/`、Makefile、队友内容或正式报告正文。不能只展示配置代码而不实际保存和验证。观察时允许设置断点及继续运行，不通过修改客体 PC、`satp`、页表项或内核指令制造预期结果。

**输出要求：** 交付可直接使用的会话准备脚本、宿主和客体配置、三终端指引、报告草稿及验收记录。验证用户将实际使用的配置，而不只验证另一套自动流程。截图限于宿主源码断点、客体内核状态等关键证据；截图由用户提供，收到前只保留文字占位。明确区分环境已经联通、用户现场截图尚缺、页表遍历尚未完成三个状态。

### [RELY] 上下文信息

- 手册：`E:/Grade3_1/操作系统/lab_book/手册完整版.html` 中的 `lab2/lab2_4_qemu_mmu_debug.html`。它以 QEMU 4.1.1 为示例，本机实际是 QEMU 7.0.0，函数位置与构建选项以本机为准。
- QEMU 源码：`/home/smile/src/qemu-7.0.0`；本次选择 `build/qemu-system-riscv64`。实际构建为 `-g -O2`，具备 DWARF 调试信息，部分变量可能因优化不可读。
- 宿主 GDB：`/usr/bin/gdb`，12.1，目标为 x86-64 QEMU 进程。客体 GDB：`/home/smile/toolchains/riscv-elf-toolchains/bin/riscv64-unknown-elf-gdb`，`16.3.90.20250610-git`，目标为 RV64 内核。
- 复用内核：`附加材料/阶段3_BestFit/kernel_tree/bin/kernel` 与 `ucore.img`。前者提供符号，后者由 QEMU 加载。开始前核对正式源码与阶段三已测源码的哈希；不要将裸镜像当作符号 ELF。
- 当前 ELF 中 `kern_init=0xffffffffc02000ee`，位于 `0xffffffffc0200000` 页；根表 VA 为 `0xffffffffc0205000`。这些是当前构建值，不是接口固定常量，配置应读取 ELF 符号确定 C 入口和代码页。
- QEMU 7.0.0 中目标入口为 `riscv_cpu_tlb_fill`，可读到参数 `address`、`access_type` 等，并定位到 `target/riscv/cpu_helper.c:1184`。实际调用路径必须从断点栈取得，不能套用其他版本。
- 当前 `ptrace_scope=1`。宿主 GDB 直接创建并调试 QEMU 子进程，保留现有系统策略。QEMU 的 `-S` 暂停模拟 CPU，`-gdb` 提供客体调试连接；这与宿主断点暂停整个 QEMU 进程不同。

### [GUARANTEE] 交付清单

本阶段没有新增内核公开接口。脚本现有入口为 `main()`，不改变实验工程接口；内部辅助函数只服务于附加材料中的调试工作。

- `prepare_session.py`：核对源码与文件，选择空闲回环端口，实际生成 `会话/host.gdb`、`guest.gdb`、`session.json` 和串口日志文件；准备动作本身不启动 QEMU。
- `verify_dual_gdb.py`：真实启动两个 GDB，验证宿主源码断点、继续及客体 C 入口断点，保存原始日志与结构化结果，并清理自己的进程。
- `verify_interactive_configs.py`：运行交付给用户的两份原样交互配置，验证断点接力和退出过程。
- `三终端操作与截图指引.md`：明确每个终端的职责、启动次序、暂停时切换到哪个窗口、两张截图应包含的内容及退出步骤。
- `双GDB环境报告草稿.md`：记录实际版本、手册适配、关键命令、实际观察与问题，沿用既有报告编号，不虚构完整页表遍历。
- `记录/` 下的环境记录、宿主与客体日志、两份验证 JSON，以及本阶段验收 JSON；保留初次失败记录，不能用期望值覆盖失败结果。
- 本文件：四段式规范提示词及下方的真实 Answer；用户截图待收到后再按约定归档。

### [SPECIFICATION] 行为规格

**会话准备。**

- Pre-Condition：已测 ELF 和镜像存在，正式源码哈希与已测版本一致，两个调试器和 QEMU 可用，输出目录在附加材料内。
- Post-Condition：宿主加载 QEMU，客体加载内核 ELF；二者使用同一会话约定的端口和镜像。生成的串口文件可供终端 1 跟踪，准备结束不留下新的运行中 QEMU。
- Case：已有本阶段活跃会话时拒绝覆盖；旧 PID 文件须核对实际进程命令行，不能仅凭文件存在就认为进程存活。端口被占用时选择其他预设空闲端口，均占用则明确报错，不结束无关进程。源码变化或依赖缺失时停止并说明，不沿用过期镜像。

**宿主 GDB。**

- Pre-Condition：QEMU 调试符号与当前源码匹配。宿主作为 QEMU 的父调试进程启动它；模拟 CPU 以 `-S` 等待客体调试器。
- Post-Condition：在内核高地址代码页的取指翻译入口停止，能显示目标客体地址、访问类型、真实调用栈和 QEMU 源码。设置 `SIGPIPE` 为不停止、不打印、继续传递，并核对配置。
- Case：断点条件同时约束代码页和 `MMU_INST_FETCH`，避免在固件的无关访问上反复停止。代码页第一次访问可能先于 `kern_init`，因此不强求宿主命中地址等于 C 入口；以实际记录解释结果。优化变量不可读时如实标明，不填写推测值。

**客体 GDB 与断点接力。**

- Pre-Condition：宿主已运行 QEMU 且 stub 可连接，客体架构为 RV64，断点地址由当前 ELF 得到。
- Post-Condition：客体继续后先触发宿主翻译入口；宿主禁用演示断点并继续后，客体到达 `kern_init` 第一条指令，读取 PC、SP、`satp`、当前指令和相关根表项。
- Case：宿主暂停时客体等待属于预期依赖，必须先继续宿主；不能在等待时反复重启或人为跳过断点。连接错误、端口冲突或未命中应保留错误并排查，不直接输出 PASS。

**证据与结束。**

- Post-Condition：同时保留原始日志和摘要；源码断点、客体断点及正常退出均有实际验证。正式 code/report 的文件清单与哈希在阶段前后相同。
- Requirements：结束时客体先 detach/quit，再中断宿主并结束其 QEMU 子进程，最后退出串口查看。用户只需截取两处关键观察；未收到截图时不得创建假图片或宣称现场采集完成。本阶段只证明联动环境和慢路径入口，不能据此宣称已读完各级 PTE 或算出本次最终物理地址。

### Answer：实际交付与迭代

复用并验证现有 QEMU 7.0.0 调试构建，采用宿主 GDB 创建 QEMU 子进程、客体 GDB 连接 stub。自动验证和原样交互配置通过；用户现场取得源码断点与 C 入口两图。实际辅助脚本问题包括 MI 异步中断设置和参数引号不匹配，修正后验证通过；现场多行粘贴也曾把 continue 当成断点编号，逐条输入后恢复。没有专门复现 SIGPIPE 或被拒绝的 attach，报告不编造这些故障。详见个人报告 4.8。

## P7 QEMU 页表翻译单步

### [PROMPT]任务说明

**任务：** 仔细阅读 Lab2 手册“分支任务：gdb 调试页表查询过程”，完成韩羽宸负责的地址翻译与页表单步。沿用已验证的双 GDB 环境，选择当前内核实际会执行的一次取指或访存，在 QEMU 源码中走完页表翻译，解释关键调用路径、分支、PTE 读取、有效性与权限判断、叶项识别及物理地址形成。给出可以复现的操作和实际证据，再逐步引导用户现场操作与截图。

**操作要求：** 以本机 QEMU 7.0.0 源码、实际构建和已测内核为准，不照搬手册示例版本的函数行号。复用符合条件的调试构建，不覆盖安装、不修改系统附加权限。只调试自己创建的 QEMU 子进程，不影响其他会话。

本阶段不得修改正式 `code/`、Makefile、正式报告正文或队友模块，也不得修改 QEMU 源码来让实验更容易通过。必须实际保存配置、脚本、原始日志和 Markdown 材料，不能只给出示意代码。调试中允许设置断点、继续和真实单步，不手改客体 PC、`satp`、PTE、指令或内存来制造预期结果。

**输出要求：** 在 `附加材料/阶段7_QEMU页表翻译` 中交付源码单步记录、独立地址计算核对、用户可用的配置、分步指引、报告草稿及验收记录。报告内容丰富但解释平实，沿用既有章节编号，以段落展开。用户自行截图；暂未收到的图片用文字占位，不伪造终端截图。本轮重点是页表翻译，TLB 查询与 Bare/Sv39 对比按原分工由队友继续。

### [RELY] 上下文信息

- 手册路径：`E:/Grade3_1/操作系统/lab_book/手册完整版.html`，重点为 `lab2/lab2_4_qemu_mmu_debug.html` 的调试要求 1、2 及本人问题记录。原始三人分工位于 `附加材料/lab2三人分工.md`。
- QEMU 源码路径为 `/home/smile/src/qemu-7.0.0`，选用 `build/qemu-system-riscv64`，已有调试信息，实际为 `-g -O2`。相关源码为 `target/riscv/cpu_helper.c`。
- 宿主 GDB 为 `/usr/bin/gdb`，目标是 x86-64 QEMU；客体 GDB 为 `/home/smile/toolchains/riscv-elf-toolchains/bin/riscv64-unknown-elf-gdb`，目标是 RV64 ucore。既有双 GDB 传输和交互控制代码位于阶段六附加材料，可读取复用，不覆盖原文件。
- 已测内核位于 `附加材料/阶段3_BestFit/kernel_tree/bin/kernel` 和 `ucore.img`。前者提供符号，后者被加载。正式源码须与阶段三已测哈希一致，QEMU 与 ELF 也须与本阶段探查版本一致。
- 本机 `get_physical_address` 的实际函数签名为：

```c
static int get_physical_address(CPURISCVState *env, hwaddr *physical,
                                int *prot, target_ulong addr,
                                target_ulong *fault_pte_addr,
                                int access_type, int mmu_idx,
                                bool first_stage, bool two_stage,
                                bool is_debug);
```

- 普通单阶段调用来自 `riscv_cpu_tlb_fill` 的 1278～1279 行。RV64 读 PTE 的语句位于 923 行，A/D 是否需要写回的判断位于 985 行，物理地址赋值位于 1033～1035 行。这些位置只适用于当前已核对源码；布局变化应停止复核。
- 当前 `kern_init` 位于 `0xffffffffc02000ee`，断点按其所在代码页选取，而不假定每次会精确翻译 C 入口地址。启动页表根项可以直接是 1 GiB 叶项，不能强行展示不存在的下层页表。
- 需要使用的实际上下文字段包括 `env->satp`、`env->priv`、`addr`、访问类型、阶段标志，以及查表时生成的 `base`、`levels`、`ptidxbits`、`ptesize`、`i`、`ptshift`、`idx`、`pte_addr`、`pte`、`res`、`ppn`、`updated_pte`、输出指针和调用者返回值。

### [GUARANTEE] 交付清单

本阶段不改变上面的 QEMU 函数签名或任何内核公开接口。Python 调试辅助函数属于附加材料，不将 C 的 `static` 要求机械套到 Python；它们不向实验工程引入新接口。

- `probe_walk.py` 的 `main()`：在真实双 GDB 中逐次执行源码 `next`，记录停点与变量、实际物理地址和客体最终断点，清理自己启动的进程。
- `summarize_walk.py` 的 `main()`：从原始记录提取观察值，独立计算根表、索引、PTE 地址与大页物理地址，交叉核对实际返回结果；保留不可读值，不能补造。
- `prepare_session.py` 的 `main()`：核对依赖和版本，在本阶段生成 `会话/host.gdb`、`guest.gdb`、串口文件及元数据；准备动作不启动 QEMU。
- `walk_commands.py`：在宿主 GDB 注册 `walk-info`、`walk-to-load`、`walk-read`、`walk-check`、`walk-result`、`walk-return`，按确定的状态顺序执行真实单步与只读观察。内部辅助函数只服务于该 GDB 会话。
- `verify_interactive_configs.py` 的 `main()`：原样执行交付配置与五段操作，确认能完成页表翻译、释放客体到 C 入口并正常退出，保存验证日志。
- `单步操作与学习指引.md`：三终端步骤、各段单步的学习要点、两张关键截图范围、退出方法及与队友衔接的共同目标。
- `QEMU页表翻译报告草稿.md`：对应既有 4.8、5.5、6.1，解释真实路径、计算与边界，保留用户截图位置。
- 本文件、`阶段7验收记录.json` 以及 `记录/` 下的原始数据、简明轨迹、计算核对和原样配置验证结果。

### [SPECIFICATION] 行为规格

**观测对象与入口。**

- Pre-Condition：已核对当前源码、QEMU、ELF 和镜像，上一轮用户会话已正常退出，新的会话端口可用。
- Post-Condition：条件断点命中实际内核代码页的取指请求，记录 `addr`、`satp`、特权级和访问类型；限定 `first_stage=true`、`two_stage=false`、`is_debug=false`，不把调试器自己读取内存的翻译当作 CPU 取指。
- Case：若版本、源码布局或镜像变化则停止；若已有本阶段活跃会话则不覆盖；端口冲突不能通过结束其他进程处理。实际地址和当前 ELF 不一致时先排查，不人工修改寄存器凑数。

**`walk-to-load` 和 `walk-read`。**

- Pre-Condition：在目标 `get_physical_address` 的入口停止，GDB 脚本状态与当前停点一致。
- Post-Condition：前者逐次 `next` 到真实 PTE 读取之前，记录根表、层数、索引和 PTE 物理地址；后者只执行一次 `next` 完成读调用，再读取 PTE 和内存事务结果。每次单步保存真实源文件、行号和宿主 PC。
- Case：关键变量不可读、提前离开函数、到达其他源码行或读取失败时明确报错并保存现场；不能打印预置 PTE 或 PASS。客体物理地址必须由 QEMU 的模拟地址空间访问，不能当作普通宿主指针。

**`walk-check`。**

- Pre-Condition：实际 PTE 已读出，内存事务成功，尚未越过合法性和权限检查。
- Post-Condition：通过真实 `next` 经过有效性、保留编码、叶项、U/S 权限、大页对齐与访问权限检查，停在 A/D 更新判断处。原始轨迹与从实际值解码的标志位分开呈现。
- Case：根项是合法叶项时按实际层级终止，不制造第二、第三层读取；目录项继续遍历、无效项失败等路径可以结合源码解释，但未执行的路径不能记为实测。当前任务针对已核对启动大页，若出现不同映射结构应停止并调整观察方案。

**`walk-result` 和 `walk-return`。**

- Pre-Condition：已完成本次叶项的检查，仍在目标函数的同一次调用中。
- Post-Condition：单步到物理地址赋值之前，再执行一次 `next`，读取实际输出；使用本轮 PPN、VA 和剩余索引位独立计算并比较。最后单步返回调用者，记录返回码、物理地址和保护位。
- Case：不能把 1 GiB 页的偏移误作最低 12 位；当前未使用 NAPOT，出现扩展路径时不强行套简化式。计算与真实输出不一致则报错并保留记录。单步行号因优化跳到共享返回或内联头文件时，结合状态与变量解释，不只凭行号推断分支。

**证据和职责范围。**

- Post-Condition：原始逐步记录、独立核对和原样交互验证都存在，正式代码和报告正文不变。用户的两张原图收到后再归档，不提前创建或宣称现场采集完成。
- Requirements：说明“翻译函数成功返回”与后续目标地址 PMP 检查、TLB 填充的关系。本轮不替代队友的 TLB 命中/未命中或 Bare/Sv39 实测。脚本不得修改客体寄存器、页表或指令；结束只清理自己创建的进程。

### Answer：实际交付与迭代

真实访问 VA=0xffffffffc0200046，satp=0x8000000000080205，根索引511，PTE地址0x80205ff8，读得0x200000cf。根级1 GiB叶项通过检查，实际PA=0x80200046，与独立计算一致，返回ret=0、prot=7。自动与原样配置各执行52次 next；用户现场也完成读表、检查、地址形成和返回，提供两张原图。用户确认会话结束；现场日志没有保留本轮再次到C入口的成功画面，该后续步骤以自动验证及第六阶段现场图支撑，不补造。本轮优化行号/变量可见性是实际理解难点，没有新的内核算法错误。详见个人报告 4.8、5.5。
