# Lab2 提示词汇总

成员：**2410673 韩羽宸、2410933 马禹翔、2410936 林子媛**。

本文件按成员和任务汇总 22 组提示词，与 [Lab2 小组实验报告](report.md) 配套。每组正文依次采用 **[PROMPT] 任务说明、[RELY] 上下文信息、[GUARANTEE] 交付清单、[SPECIFICATION] 行为规格**，之后另列实际结果或理解修正，不将结果混入生成前的要求。

提示词中的路径、版本、操作限制和提交要求属于原实施阶段的上下文，不是本次整理要重复执行的命令。完整提示词集中于本文件；报告保留各练习的 SPEC、核心实现、迭代和证据，并链接到对应记录。原始个人报告与马禹翔的独立提示词文件保留供核对。

## 提示词索引

| 编号 | 负责人 | 任务 | 对应报告 |
| --- | --- | --- | --- |
| [HYC-P1](#hyc-p1) | 2410673 韩羽宸 | 物理内存探测与页初始化 | 4.2 |
| [HYC-P2](#hyc-p2) | 2410673 韩羽宸 | Best Fit 完整模块与接入 | 4.5 |
| [HYC-P3](#hyc-p3) | 2410673 韩羽宸 | 启动分页过渡修复 | 4.1 |
| [HYC-P4](#hyc-p4) | 2410673 韩羽宸 | 原有检查与补充验证 | 5.2 |
| [HYC-P5](#hyc-p5) | 2410673 韩羽宸 | 启动原理、六问与物理内存 Challenge | 4.3 |
| [HYC-P6](#hyc-p6) | 2410673 韩羽宸 | 双 GDB 调试环境 | 4.8 |
| [HYC-P7](#hyc-p7) | 2410673 韩羽宸 | QEMU 页表翻译单步 | 4.8 |
| [MYX-P1](#myx-p1) | 2410933 马禹翔 | `pmm_init()`、`page_init()` 与 `pmm_manager` 的调用关系 | 4.1 |
| [MYX-P2](#myx-p2) | 2410933 马禹翔 | 物理页、`struct Page`、`flags` 与 `property` 的关系 | 4.1 |
| [MYX-P3](#myx-p3) | 2410933 马禹翔 | `default_init_memmap()` 是否在探测占用页 | 4.4 |
| [MYX-P4](#myx-p4) | 2410933 马禹翔 | `default_init_memmap()` 如何把空闲块插入链表 | 4.4 |
| [MYX-P5](#myx-p5) | 2410933 马禹翔 | Buddy System 的原理、实现与存在意义 | 4.6 |
| [MYX-P6](#myx-p6) | 2410933 马禹翔 | 设计 `buddy_pmm.h` 公共接口 | 4.6 |
| [MYX-P7](#myx-p7) | 2410933 马禹翔 | 实现 `buddy_pmm.c` 完整管理模块 | 4.6 |
| [MYX-P8](#myx-p8) | 2410933 马禹翔 | 第二份 Prompt——生成并补全 Buddy 边界测试 | 4.6 / 5.3 |
| [MYX-P9](#myx-p9) | 2410933 马禹翔 | 完成个人负责内容并进行 Buddy 内核运行验证 | 5.3 |
| [MYX-P10](#myx-p10) | 2410933 马禹翔 | 整理并提交个人代码、报告、截图与 AI 交流记录 | 5.1 / 6.3 |
| [LZY-P1](#lzy-p1) | 2410936 林子媛 | SLUB 实现 | 4.7 |
| [LZY-P2](#lzy-p2) | 2410936 林子媛 | SLUB 独立测试 | 5.4 |
| [LZY-P3](#lzy-p3) | 2410936 林子媛 | 理解页、描述符与 SLUB 层次 | 3.1 / 4.1 |
| [LZY-P4](#lzy-p4) | 2410936 林子媛 | 分析 Buddy 与 SLUB 的契约 | 4.7 |
| [LZY-P5](#lzy-p5) | 2410936 林子媛 | 对比 Bare/Sv39 和软件 TLB | 4.8 / 5.5 |

---

**2410673 韩羽宸：** 七组提示词按韩羽宸的实际需求、工程定义和执行结果整理，并非逐字聊天记录。操作范围描述对应当时阶段；临时验证与调试助手在本地附加材料运行。

<a id="hyc-p1"></a>

## HYC-P1：物理内存探测与页初始化

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

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

### [GUARANTEE] 交付清单

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

### 结果与后续调整

在实际 `pmm.c` 中补全 `page_init()`，标记 `2410673`。实现了 DTB 结果校验、管理上限、描述符布局、保留区及完整空闲页登记。第一次完整执行 13 个初始化用例均通过，未观察到需要修正的初始化算法错误。初次测试驱动因调用只有声明的 `sbi_shutdown()` 链接失败，仅在附加材料中改为局部退出调用；没有修改正式 SBI 模块。后续完整内核中任务一 PASS，详见个人报告 4.2、5.1、5.2。

<a id="hyc-p2"></a>

## HYC-P2：Best Fit 完整模块与接入

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

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

### [GUARANTEE] 交付清单

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

### 结果与后续调整

完整 Best Fit 模块及管理器切换已实现，保留原有 `basic_check()`、`best_fit_check()`。原有检查得到 6/6，任务二 PASS，适用评分 20/20；补充 10 组行为验证及 3000 次独立模型操作通过。算法查找、拆分、左右合并和计数均有实际验证，没有虚构额外算法 bug。首次集成失败来自原启动分页过渡，另以 P3 处理。详见个人报告 4.5 和 5.2。

<a id="hyc-p3"></a>

## HYC-P3：启动分页过渡修复

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

**任务：** 修复实际 `code/kern/init/entry.S` 中从 Bare 切换到 Sv39 后无法继续取指的问题，使完整 Lab2 内核能够进入 `kern_init()` 并执行内存管理检查。

**操作要求：** 修改实际启动汇编，标明 2410673。保留现有高地址链接、虚实偏移、根页表、OpenSBI 参数保存和原有 C 入口，不改动分配器或已有测试。只增加启动过渡所需的最小映射和控制流，进入高地址后必须移除临时映射并刷新相关地址翻译状态。

**输出要求：** 解释原故障的实际证据、临时映射覆盖的区间、何时跳转和何时清理。运行完整内核并用 GDB 核对最终页表，不得只根据输出猜测修复有效。

### [RELY] 上下文信息

原始完整内核已成功编译，但没有输出 C 初始化信息。GDB 单步的实际结果为：`satp = 0x8000000000080205`、`scause = 0xc`、`stval = 0x80200038`。对应位置为写入 `satp` 后的 `sfence.vma` 指令，说明低地址取指产生了 instruction page fault。

原始代码以 `0xffffffffc0200000` 为链接起点，物理加载地址为 `0x80200000`，偏移为 `0xffffffff40000000`。根页表包含 512 个 8 字节项，原来仅第 511 项有效，其值为 `(0x80000 << 10) | 0xcf`，将高地址区域映射到从 `0x80000000` 起的 1 GiB 物理区。开启分页不会自动改变当前 PC 的数值。

根页表第 2 项对应从 `0x80000000` 起的低地址 1 GiB 区域。若临时填入同样的大页叶项，则启动代码当前 PC 所处的物理区可以继续按同值虚拟地址取指。实际根表符号为 `boot_page_table_sv39`，C 入口为 `kern_init`，栈顶为 `bootstacktop`。

### [GUARANTEE] 交付清单

- 保持入口符号 `kern_entry`、根表符号 `boot_page_table_sv39`、`boot_hartid`、`boot_dtb`、`bootstack` 和 `bootstacktop` 的职责。
- 保持页表为一页、512 项，不改变最终高地址映射和原有 C 接口。
- 可增加仅供汇编内部使用的跳转标签，例如 `.Lhigh_entry`，不新增公共函数。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 内核在 `0x80200000` 开始执行，处于启动阶段，尚未使用 C 栈，OpenSBI 的 hartid 与 DTB 参数仍可保存。页表尚未作为运行中共享结构使用。

**Post-Condition：** 控制流安全进入高地址，再使用高地址栈和 C 入口。进入 C 初始化后，第 2 个根页表项为零，第 511 项仍为 `0x200000cf`，`satp` 处于 Sv39 且仍指向原根表。

**Case 1：切换时低地址 PC。** 开启 Sv39 后，到高地址跳转之前的指令必须有有效取指映射，不能依赖未设置的异常处理程序进行恢复。

**Case 2：已经进入高地址。** 临时低地址映射应被清除，之后执行 `sfence.vma`；清除动作和后续指令须依靠高地址映射继续执行。

**Requirements：** 保留失败启动日志和单步寄存器证据；修复后既要通过完整内核检查，也要用 GDB 观察实际 PC、`satp` 和两个相关根页表项。

### 结果与后续调整

真实故障为 `scause=0xc`、`stval=0x80200038`：写入 satp 后 PC 仍低，而原页表只映射高地址。增加根表第 2 项的临时恒等映射，跳到高地址后清零该项并执行 sfence.vma，再设置栈与进入 C。修复后完整内核和原有评分通过；现场启动观察确认进入 C 后 root[2]=0、root[511]=0x200000cf。详见个人报告 4.1 及启动截图。

<a id="hyc-p4"></a>

## HYC-P4：原有检查与补充验证

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

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

### 结果与后续调整

原有检查、两项任务检查及评分保持不变。初始化 13 个用例通过；Best Fit 10 组检查、3000 次模型操作及累计 3060 次状态核对通过；原有评分 20/20。用户现场截图已归档，正文采用完整运行和评分图，补充测试图以链接保留。异常初始化用例的 PASS 表示捕获预期终止，不是无效输入初始化成功；评分不覆盖 Buddy、SLUB 或全部调试。详见个人报告第五章。

<a id="hyc-p5"></a>

## HYC-P5：启动原理、六问与物理内存 Challenge

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

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

### 结果与后续调整

结合真实代码完成分页设计六问、启动与 DTB 主线、物理内存范围 Challenge 及 Best Fit 改进分析。GDB 记录了启动六个时刻，用户提供现场图。Challenge 分析 DTB、固件/引导程序内存图、保留区和对齐；本工程未实现 E820/UEFI 探测。观察脚本初次因 dump binary memory 的文件名引号导出失败，仅修改附加材料的路径格式后重跑成功。详见个人报告 3、4.1、4.3。

<a id="hyc-p6"></a>

## HYC-P6：双 GDB 调试环境

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

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

### 结果与后续调整

复用并验证现有 QEMU 7.0.0 调试构建，采用宿主 GDB 创建 QEMU 子进程、客体 GDB 连接 stub。自动验证和原样交互配置通过；用户现场取得源码断点与 C 入口两图。实际辅助脚本问题包括 MI 异步中断设置和参数引号不匹配，修正后验证通过；现场多行粘贴也曾把 continue 当成断点编号，逐条输入后恢复。没有专门复现 SIGPIPE 或被拒绝的 attach，报告不编造这些故障。详见个人报告 4.8。

<a id="hyc-p7"></a>

## HYC-P7：QEMU 页表翻译单步

**负责人：2410673 韩羽宸**

### [PROMPT] 任务说明

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

### 结果与后续调整

真实访问 VA=0xffffffffc0200046，satp=0x8000000000080205，根索引511，PTE地址0x80205ff8，读得0x200000cf。根级1 GiB叶项通过检查，实际PA=0x80200046，与独立计算一致，返回ret=0、prot=7。自动与原样配置各执行52次 next；用户现场也完成读表、检查、地址形成和返回，提供两张原图。用户确认会话结束；现场日志没有保留本轮再次到C入口的成功画面，该后续步骤以自动验证及第六阶段现场图支撑，不补造。本轮优化行号/变量可见性是实际理解难点，没有新的内核算法错误。详见个人报告 4.8、5.5。

---

**2410933 马禹翔：** 十组记录覆盖概念澄清、Buddy 接口与实现、边界测试及交付过程。保留各阶段真实结果；实现和测试的核心提示词为 MYX-P7、MYX-P8。Git 操作要求仅记录当时任务，不代表本次统稿执行提交或推送。

<a id="myx-p1"></a>

## MYX-P1：`pmm_init()`、`page_init()` 与 `pmm_manager` 的调用关系

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 根据当前 Lab2 源码，解释内核进入 C 入口后物理内存管理的初始化顺序，判断“内核先调用 `pmm_manager`，再调用 `pmm_init()`”是否正确，并说明 `pmm_init()` 与 `page_init()` 的包含及调用关系。

**操作要求：** 只进行源码分析和概念澄清，不修改内核源代码；区分管理框架入口、具体分配器初始化、物理内存发现与空闲区登记。

**输出要求：** 给出真实调用链，并解释每个阶段建立了什么状态、为什么顺序不能颠倒。

### [RELY] 上下文信息

- `kern/init/init.c` 中 `kern_init()` 调用 `pmm_init()`。
- `kern/mm/pmm.c` 中 `pmm_init()` 依次调用 `init_pmm_manager()`、`page_init()` 和 `check_alloc_page()`。
- `init_pmm_manager()` 先令全局指针 `pmm_manager` 指向具体管理器，再调用 `pmm_manager->init()`。
- `page_init()` 负责发现可管理物理内存、建立 `Page` 描述符，并通过 `init_memmap()` 调用 `pmm_manager->init_memmap(base, n)` 登记初始空闲页。

### [GUARANTEE] 交付清单

- 明确外层入口是 `pmm_init()`，不是 `pmm_manager` 主动调用 `pmm_init()`。
- 区分 `pmm_manager->init()` 与 `page_init()` 的职责。
- 说明 `page_init()` 是 `pmm_init()` 的内部步骤。
- 给出从 `kern_init()` 到具体分配器的完整调用链。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 内核已经建立启动页表、进入 `kern_init()` 并完成此前的基本初始化，但物理页分配器尚未初始化。

**Post-Condition：** `pmm_init()` 完成后，当前分配器已选定，其内部空闲数据结构已初始化，可用物理页已登记，并已运行分配器检查。

**Case 1——选择并初始化算法：** `init_pmm_manager()` 设置 `pmm_manager` 指针并调用具体算法的 `init()`，此时只初始化链表、计数器等算法内部状态，尚未加入真实空闲页。

**Case 2——发现并登记内存：** `page_init()` 根据物理内存布局建立页描述符、排除已占用区域，并将剩余完整空闲页通过 `pmm_manager->init_memmap()` 交给当前算法。

**Requirements：** 不把 `page_init()` 与 `pmm_manager->init()` 混为一谈；不把函数指针表描述成会主动启动的对象。

### 结果与后续调整

原先“先调用 `pmm_manager` 启动内存分配模块，再调用 `pmm_init()`”的顺序需要修正。真实顺序是 `kern_init()` 先调用总入口 `pmm_init()`；`pmm_init()` 内部先选择具体的 `pmm_manager` 并初始化算法内部状态，再调用 `page_init()` 发现和登记真实可用物理页，最后运行检查。`page_init()` 是 `pmm_init()` 的组成步骤，而不是与它并列或先于它执行的外部入口。

<a id="myx-p2"></a>

## MYX-P2：物理页、`struct Page`、`flags` 与 `property` 的关系

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 澄清 `Page` 是否就是物理页、`flags` 是否是物理页描述符，并解释为什么 `property` 只在空闲连续块的块头有效。

**操作要求：** 依据当前工程中的 `struct Page`、页面状态宏和 First Fit 实现进行概念分析，不修改源码。

**输出要求：** 区分真实物理页、页描述符、描述符字段和空闲块元数据，并通过连续空闲页示例说明 `property` 的含义。

### [RELY] 上下文信息

- `kern/mm/memlayout.h` 定义 `struct Page { int ref; uint64_t flags; unsigned int property; list_entry_t page_link; }`。
- 每个 `struct Page` 描述一个物理页；真实物理页是物理内存中的一段固定大小空间。
- `flags` 通过 `PG_reserved`、`PG_property` 等位表示页面状态。
- First Fit 只把空闲连续块的首个 `Page` 挂入 `free_list`，并在该块头的 `property` 中保存块包含的页数。
- 只有 `PageProperty(page)` 为真时，当前实现才把该页的 `property` 解释为空闲块长度。

### [GUARANTEE] 交付清单

- 明确 `Page` 在源码语境中通常指 `struct Page` 描述符，而不是真实 4 KiB 内存内容。
- 明确 `flags` 只是描述符的一个字段，不是完整描述符。
- 解释块头、内部页和链表节点之间的关系。
- 说明非块头页面的 `property` 可能为零或残留旧值，但没有 `PG_property` 时不应使用。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 已知物理内存按页划分，内核使用 `pages[]` 数组为每个可管理物理页保存一个 `struct Page`。

**Post-Condition：** 能够根据 `struct Page *`、`flags` 和 `property` 判断某个描述符是否为空闲块头，以及该块包含多少页。

**Case 1——空闲块头：** `PG_property` 置位，`property` 表示从该页开始的连续空闲页数，`page_link` 可挂入空闲链表。

**Case 2——空闲块内部页：** 不设置 `PG_property`，不单独挂入链表，`property` 不具有块长度语义。

**Case 3——已分配或保留页：** 不应按空闲块头解释；是否保留由 `PG_reserved` 等状态位判断。

**Requirements：** 判断 `property` 是否有效时必须同时检查块头标志，不能仅凭数值非零判断。

### 结果与后续调整

`struct Page` 是物理页的元数据描述符，`flags` 只是其中记录状态的一部分。真实物理页保存实际字节，描述符保存在独立的 `pages[]` 数组中。连续空闲块在链表中只需要一个节点，因此只在首个描述符上设置 `PG_property` 并用 `property` 记录整块页数；内部页不独立代表空闲块，其 `property` 即使存在旧值也应被忽略。

<a id="myx-p3"></a>

## MYX-P3：`default_init_memmap()` 是否在探测占用页

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 解释 `default_init_memmap()` 开头遍历 `[base, base+n)` 的循环是否用于探测已占用物理页，并判断把每页 `property` 清零是否表示后续不存在连续空闲块。

**操作要求：** 结合 `page_init()` 与 `init_memmap()` 的职责边界，以及循环之后建立块头的代码进行分析；不修改源码。

**输出要求：** 说明 `assert(PageReserved(p))` 的准确含义、清零各字段的目的，以及循环结束后连续空闲块如何建立。

### [RELY] 上下文信息

- `page_init()` 先完成物理内存范围判断、排除实际占用区域和页面对齐，再把已经确认可用的 `[base, base+n)` 传给 `init_memmap()`。
- `init_memmap()` 转发到当前管理器的 `default_init_memmap(base, n)`。
- `default_init_memmap()` 进入循环时要求每个描述符仍带有 `PG_reserved`，表示这些页尚未交给分配器管理。
- 循环之后执行 `base->property = n`、`SetPageProperty(base)`、`nr_free += n`，并把 `base->page_link` 插入空闲链表。

### [GUARANTEE] 交付清单

- 明确该循环不是物理内存探测过程。
- 区分“实际被内核占用”与“描述符暂时标为 Reserved、尚未交给分配器”。
- 解释清零 `flags`、`property` 和引用计数是在初始化已确认可用的页。
- 说明整段连续空闲块在循环后由 `base` 统一表示。

### [SPECIFICATION] 行为规格

**Pre-Condition：** `[base, base+n)` 已由上层确认是可交给分配器的连续完整物理页；其描述符在全局初始化阶段暂时统一标记为 Reserved。

**Post-Condition：** 区间中每页不再保留旧状态和引用计数；`base` 成为大小为 `n` 的唯一空闲块头，整段被登记到空闲链表，空闲页总数增加 `n`。

**Case 1——可用页区间：** 断言成立，逐页清理描述符，随后以 `base` 建立连续空闲块。

**Case 2——仍包含不应释放的页：** 若某个描述符不满足预期状态，断言用于暴露上层内存布局或重复初始化错误，而不是把该页识别后跳过。

**Requirements：** 不把循环中的临时 `property=0` 当作最终状态；必须连同循环后的块头设置一起解释。

### 结果与后续调整

该循环是在初始化一段已经确认可用的物理页，而不是探测哪些页被占用。所有页先统一清除旧标志、旧块长度和引用计数；随后只在 `base` 上设置 `property=n` 和 `PG_property`，把整个 `[base, base+n)` 表示为一个连续空闲块。真正被 OpenSBI、内核或描述符数组占用的页不会被传入该函数，并继续保持保留状态。

<a id="myx-p4"></a>

## MYX-P4：`default_init_memmap()` 如何把空闲块插入链表

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 解释 `default_init_memmap()` 中根据 `list_empty()`、`base < page` 和链表尾部条件执行 `list_add()` 或 `list_add_before()` 的代码，判断其是否在把空闲页连接到链表。

**操作要求：** 结合 `free_list` 的哨兵头、`base->page_link`、`le2page()` 和地址有序要求进行分析，不修改源码。

**输出要求：** 说明空链表、插入中间、追加尾部三种情况，并明确链表节点对应单页还是空闲块头。

### [RELY] 上下文信息

- 在执行插入代码前，`base->property=n` 且 `PG_property` 已置位，因此 `base` 表示整个 `[base, base+n)` 连续空闲块。
- `free_list` 是无数据的循环双向链表哨兵头。
- `base->page_link` 是该空闲块头嵌入的链表节点。
- `le2page(le, page_link)` 从已有链表节点取得对应空闲块头的 `struct Page *`。
- `Page` 描述符位于连续的 `pages[]` 数组中，所以比较同一数组中的 `Page *` 可以反映页描述符及对应物理页的先后顺序。

### [GUARANTEE] 交付清单

- 明确只把块头挂入链表，不把块内每个页都挂入链表。
- 解释空链表时将节点加入哨兵头之后。
- 解释遇到更大地址块时插到它之前。
- 解释遍历到最后一个块时追加到尾部。
- 说明保持地址有序对释放时合并前后相邻块的作用。

### [SPECIFICATION] 行为规格

**Pre-Condition：** `base` 已被初始化为大小为 `n` 的有效空闲块头，且该块尚未在 `free_list` 中。

**Post-Condition：** `base->page_link` 恰好一次地进入 `free_list`，链表仍按块头地址递增排列。

**Case 1——空链表：** 将 `base->page_link` 插入哨兵头之后，此节点同时是首节点和尾节点。

**Case 2——应插入已有块之前：** 遍历得到已有块头 `page`，若 `base < page`，则在当前节点之前插入并结束搜索。

**Case 3——地址最大：** 若已经到达最后一个已有节点仍未找到更大块，则把新节点插到最后一个节点之后。

**Requirements：** 链表节点表示连续空闲块，而非单独空闲页；代码依赖调用者保证新区间合法且不与现有空闲块重叠。

### 结果与后续调整

该代码确实把空闲内存登记到空闲链表，但登记单位是空闲连续块。整个 `[base, base+n)` 只使用 `base->page_link` 一个节点，并根据 `base` 与已有块头的地址比较插入到正确位置。地址有序使 `default_free_pages()` 在释放块时只需检查链表前驱和后继，就能判断是否存在可合并的相邻空闲块。

<a id="myx-p5"></a>

## MYX-P5：Buddy System 的原理、实现与存在意义

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 解释 Buddy System 是什么、如何完成初始化、分配、拆分、释放与合并，并分析在已有 First Fit（default）和 Best Fit 的情况下为什么仍需要这种分配方式。

**操作要求：** 结合当前 ucore 的页级 `pmm_manager` 接口说明，不直接修改源码；同时给出 Buddy 的优势、代价和适用场景，避免把它描述为绝对优于其他算法。

**输出要求：** 通过按阶空闲链表和具体页数示例说明算法流程，并给出在本项目中实现 Buddy 模块时需要的数据结构、函数和关键不变量。

### [RELY] 上下文信息

- 当前 `pmm_manager` 提供 `init`、`init_memmap`、`alloc_pages`、`free_pages`、`nr_free_pages` 和 `check` 接口。
- First Fit 和 Best Fit 使用连续空闲块；First Fit 选择第一个足够大的块，Best Fit 选择最小的足够大块。
- Buddy 要求每个受管理块的大小为 `2^order` 页，并按 order 分别组织空闲块。
- 对非 2 次幂请求，可以事先约定向上取整到最小可容纳的 2 次幂块。
- 两个块只有在大小相同、阶相同、互为伙伴且均空闲时才能合并。

### [GUARANTEE] 交付清单

- 解释 order、按阶空闲链表和伙伴关系。
- 给出分配时向上取整、向高阶查找和逐级拆分的过程。
- 给出释放时伙伴定位和递归合并的过程。
- 分析 Buddy 相比 First Fit/Best Fit 在查找、合并和碎片方面的取舍。
- 列出本项目可能新增的 `buddy_pmm.h`、`buddy_pmm.c` 和核心接口。
- 说明 Buddy 不能取代对象级任意大小分配器，也不一定适合所有负载。

### [SPECIFICATION] 行为规格

**Pre-Condition：** 物理内存已经按页划分，`init_memmap(base,n)` 提供一段已确认可用的连续页区间。

**Post-Condition：** 初始可用区被分解成若干对齐、不越界的 2 次幂块；分配和释放保持各阶链表、块头元数据及空闲页总数一致。

**Case 1——精确阶命中：** 目标阶链表非空时直接取出一个块。

**Case 2——需要拆分：** 目标阶为空时从更高阶取块，重复分成两个低一阶伙伴，保留一半继续拆分并把另一半放入对应阶链表。

**Case 3——释放并合并：** 根据块头和 order 找到伙伴；伙伴同阶且空闲时移除伙伴、形成更高阶块并继续，否则把当前块加入本阶链表。

**Case 4——非 2 次幂请求：** 按预先约定向上取整，计数和释放均使用实际块大小，避免语义不一致。

**Requirements：** 拆分和合并本身不改变空闲总页数；只有真正分配和释放改变计数。伙伴计算必须基于统一页索引，并检查管理范围和对齐边界。

### 结果与后续调整

Buddy 将空闲内存按 `1、2、4、8……` 页的阶组织，通过逐级二分获得所需块，通过伙伴关系快速判断能否合并。它相较单链表上的 First Fit/Best Fit，能够提供规则且通常更快的查找、拆分和合并，并抑制难以合并的外部碎片；代价是非 2 次幂请求需要向上取整，可能产生内部碎片，还需要维护多条空闲链表和阶信息。因此 Buddy 是另一种时间、空间和实现复杂度取舍，而不是对现有两种算法的无条件替代。

<a id="myx-p6"></a>

## MYX-P6：设计 `buddy_pmm.h` 公共接口

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 新建 `kern/mm/buddy_pmm.h`，为后续 Buddy 物理页分配模块定义公共接口。该模块后续需要支持初始化、初始内存登记、页面分配、页面释放、空闲页计数和正确性检查，并能够通过现有 `struct pmm_manager` 接入内核。

**操作要求：** 必须修改项目中的真实文件；遵循 `default_pmm.h` 与 `best_fit_pmm.h` 的封装方式；保留 `lab2` 标记并注明实现者学号 `2410933`。头文件只暴露其他模块真正需要使用的公共符号，不公开 Buddy 的各阶链表、计数器、管理范围或内部辅助函数。

**输出要求：** 创建带有独立 include guard 的 `buddy_pmm.h`，包含 `pmm.h`，并导出 Buddy 管理器注册对象。初始化、初始内存登记、分配、释放、计数、检查函数及内部基础变量在后续 `buddy_pmm.c` 中以 `static` 形式实现，通过注册对象的函数指针提供服务。

### [RELY] 上下文信息

- `pmm.h` 已定义 `struct pmm_manager`，其中包含 `init`、`init_memmap`、`alloc_pages`、`free_pages`、`nr_free_pages` 和 `check` 函数指针。
- `default_pmm.h` 只包含 `pmm.h` 并导出 `extern const struct pmm_manager default_pmm_manager`。
- `best_fit_pmm.h` 采用相同方式导出 `best_fit_pmm_manager`。
- Buddy 的实现细节只由 `buddy_pmm.c` 使用，不应成为内核公共 API。

### [GUARANTEE] 交付清单

必须新增文件：

```c
kern/mm/buddy_pmm.h
```

必须公开以下注册对象：

```c
extern const struct pmm_manager buddy_pmm_manager;
```

后续 `buddy_pmm.c` 必须为该注册对象提供以下类型匹配的内部回调：

```c
static void buddy_init(void);
static void buddy_init_memmap(struct Page *base, size_t n);
static struct Page *buddy_alloc_pages(size_t n);
static void buddy_free_pages(struct Page *base, size_t n);
static size_t buddy_nr_free_pages(void);
static void buddy_check(void);
```

允许后续在 `buddy_pmm.c` 中新增 `static` 辅助函数、按阶空闲链表、空闲页总数和管理边界等内部变量，但不得在本头文件中暴露这些实现细节。

### [SPECIFICATION] 行为规格

#### `buddy_pmm.h`

**Pre-Condition：** 工程已有 `pmm.h`，其他管理器头文件均通过导出一个 `const struct pmm_manager` 注册对象接入。

**Post-Condition：** 任何需要选择 Buddy 分配器的源文件均可通过包含 `buddy_pmm.h` 获取 `buddy_pmm_manager` 声明；重复包含头文件不会产生重复定义；头文件自身不分配存储，也不暴露内部可变状态。

**Case 1——正常包含：** include guard 生效，`pmm.h` 提供完整的 `struct pmm_manager` 类型，外部源文件可以引用 `buddy_pmm_manager`。

**Case 2——重复包含：** 同一翻译单元多次包含时，只保留一次有效声明。

**Requirements：** 注册对象使用 `extern const` 声明；具体定义只能在一个 `.c` 文件中出现；内部回调和变量保持模块私有；不得修改现有 First Fit、Best Fit 头文件或测试函数。

### 结果与后续调整

已创建 `kern/mm/buddy_pmm.h`，使用独立 include guard，包含 `pmm.h`，并公开 `extern const struct pmm_manager buddy_pmm_manager`。原始需求中“在头文件声明初始化、登记、分配、释放、计数函数和基础变量”的范围已按现有工程封装方式收紧：这些内容属于 Buddy 内部实现，将在 `buddy_pmm.c` 中声明为 `static`，公共头文件只暴露统一管理器注册对象。

<a id="myx-p7"></a>

## MYX-P7：实现 `buddy_pmm.c` 完整管理模块

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 新建 `kern/mm/buddy_pmm.c`，从零实现符合现有 ucore `struct pmm_manager` 接口的 Buddy 物理页分配模块，包括内部状态初始化、初始空闲区登记、按阶分配与拆分、释放与递归伙伴合并、空闲页计数、基础正确性检查和管理器注册。

**操作要求：** 必须修改项目中的真实文件；保留现有 First Fit、Best Fit 和测试代码；不在本步骤修改 `pmm.c` 的默认管理器选择。新增代码保留 `lab2` 标记并注明实现者学号 `2410933`。内部函数和变量必须使用 `static`，只通过 `buddy_pmm_manager` 对外提供统一接口。

**输出要求：** 生成可独立编译的 `buddy_pmm.c`，与已创建的 `buddy_pmm.h` 配套；实现完成后在实验虚拟机中编译，记录真实编译结果和需要修正的问题。

### [RELY] 上下文信息

- `pmm.h` 提供 `struct pmm_manager`、`pages`、`npage`、`nbase`、`page2ppn()`、`Page` 状态宏和页分配通用接口。
- `memlayout.h` 定义 `struct Page`；本模块使用 `property` 保存块的实际页数，使用 `PG_property` 表示空闲块头。
- `list.h` 提供循环双向链表操作和 `le2page()`。
- `buddy_pmm.h` 已声明 `extern const struct pmm_manager buddy_pmm_manager`。
- 可用物理页以 `init_memmap(base,n)` 形式交给模块，进入函数时页面描述符仍为 Reserved。
- 非 2 次幂请求采用向上取整到最小 `2^order` 页块的规则，释放和计数使用实际块大小。

### [GUARANTEE] 交付清单

必须实现以下内部回调：

```c
static void buddy_init(void);
static void buddy_init_memmap(struct Page *base, size_t n);
static struct Page *buddy_alloc_pages(size_t n);
static void buddy_free_pages(struct Page *base, size_t n);
static size_t buddy_nr_free_pages(void);
static void buddy_check(void);
```

必须定义：

```c
const struct pmm_manager buddy_pmm_manager;
```

允许新增内部结构和辅助函数，用于按阶链表、阶计算、块插入/移除、伙伴定位、递归合并、管理范围检查和测试区间重叠检查。

### [SPECIFICATION] 行为规格

#### 内部状态

**Pre-Condition：** 模块尚未初始化或正由 `pmm_init()` 选择为当前管理器。

**Post-Condition：** 每个 order 有独立空闲链表和块计数，并维护所有阶共享的准确空闲页总数。

#### `buddy_init`

**Post-Condition：** 初始化全部阶链表，清零块计数和空闲页总数，不登记真实物理页。

#### `buddy_init_memmap`

**Pre-Condition：** `[base,base+n)` 是上层确认可用的连续完整物理页，`n>0`，描述符处于 Reserved 状态。

**Post-Condition：** 清理页面状态，将区间分解成若干按绝对物理页号对齐、大小为 2 的幂且不越界的块；块头进入对应 order 链表，空闲页总数增加 `n`。

#### `buddy_alloc_pages`

**Case 1——目标阶有空闲块：** 移除一个块并返回块头。

**Case 2——仅高阶有块：** 从最近的高阶取块，逐级拆分，把右半伙伴加入低一阶链表，直到达到目标阶。

**Case 3——无可用块：** 返回 `NULL`，不改变任何状态。

**Post-Condition：** 返回块的 `PG_property` 清除，块头 `property` 保存向上取整后的实际块大小，空闲页总数减少实际块大小。

#### `buddy_free_pages`

**Pre-Condition：** `base` 是本模块返回的有效块头，`n` 经相同规则取整后与分配时实际块大小一致，块未重复释放。

**Post-Condition：** 清理块内页状态，空闲页总数只增加一次；若同阶伙伴空闲则移除伙伴、合并并继续向高阶检查，最终块头加入对应链表。

#### `buddy_check`

必须至少验证：超出总空闲量的请求失败、两个单页伙伴可重新合并为两页块、非 2 次幂请求分别占用 4 页和 8 页、不同分配区间不重叠，以及全部释放后空闲页计数恢复。

**Requirements：** 拆分和合并不得改变空闲页总数；伙伴计算基于物理页号 XOR 块大小；合并必须检查伙伴在可管理范围内、带有 `PG_property` 且块大小相同；不得越过 `nbase..npage`；不得修改现有检查函数。

### 结果与后续调整

已生成 `buddy_pmm.c` 第一版，包含按阶链表、请求阶计算、对齐拆分、逐级分配、伙伴递归合并、实际块大小记录、空闲页计数、自检和注册结构。首次完整 `make` 被尚未实现的 `best_fit_pmm.c` 阻断，错误为其中缺少 `free_list` 和 `nr_free`，并非 Buddy 模块错误；随后单独执行 `make obj/kern/mm/buddy_pmm.o`，Buddy 源文件编译通过。人工审查发现原自检对全局空闲布局存在不必要假设，已调整为先分配一个 8 页对齐块并建立隔离测试区，测试完成后恢复原有各阶链表和计数并归还该块。本阶段尚未把 `pmm.c` 的当前管理器切换为 Buddy，因此普通启动仍不会执行运行级 `buddy_check()`；完整内核运行需等待 `page_init()` 和 Best Fit 主责部分可构建，或在独立实验副本中临时接入。

<a id="myx-p8"></a>

## MYX-P8：第二份 Prompt——生成并补全 Buddy 边界测试

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 针对当前 `kern/mm/buddy_pmm.c` 已实现的 Buddy 分配器，审查并补全 `buddy_check()` 测试。测试必须直接使用真实内部接口和状态约定，验证按阶分配、拆分、伙伴判断、递归合并、非 2 次幂向上取整、内存不足、区间不重叠、非伙伴相邻块以及不规则初始内存区登记。

**操作要求：** 必须修改项目中的真实 `buddy_pmm.c`；不得修改 First Fit、Best Fit 的原有检查函数，不得伪造运行输出。测试应先从真实 Buddy 管理器中取得一个对齐测试区，暂存并隔离真实各阶链表，在受控环境中运行，最后清空测试区、恢复原管理状态并归还测试页，保证测试前后总空闲页数一致。

**输出要求：** 扩展 `buddy_check()`，为每类行为设置明确断言；代码需在现有 `-Werror` 编译选项下通过。若完整工程被其他未完成模块阻断，应单独编译 `buddy_pmm.o`，并如实记录阻断原因与 Buddy 自身编译结果。

### [RELY] 上下文信息

- 当前实现使用 `buddy_free_areas[order]` 保存各阶空闲链表及块数，使用 `buddy_nr_free` 保存空闲页总数。
- `buddy_alloc_pages(n)` 将非 2 次幂请求向上取整，并在返回块头的 `property` 中保存实际块大小；返回块不设置 `PG_property`。
- `buddy_free_pages(base,n)` 使用相同取整规则，校验实际块大小，并通过 XOR 查找同阶伙伴。
- `buddy_find_free_buddy()` 只有在伙伴处于范围内、带有 `PG_property` 且 `property` 等于当前块大小时才返回伙伴。
- 测试可以使用模块内部 `static` 函数和变量，但不得将其暴露到公共头文件。

### [GUARANTEE] 交付清单

测试必须覆盖：

1. 请求超过总空闲页数时返回 `NULL` 且状态不变。
2. 两个单页分配来自同一 order-1 块时互为伙伴，释放后能够恢复为两页块。
3. 2 页精确匹配。
4. 两个 3 页请求分别占用 4 页且区间不重叠。
5. 两个 5 页请求分别占用 8 页，耗尽 16 页测试区后额外请求失败。
6. 四个连续 2 页块中，中间两个地址相邻但不是伙伴；在各自真实伙伴仍分配时，它们不能错误合并。
7. 以不同于地址顺序的次序释放后，真实伙伴能够逐级合并并恢复完整 16 页块。
8. 仅登记 `[arena+1, arena+14)` 共 13 页的不对齐、非 2 次幂区间时，应分解为对齐且不越界的块。
9. 从该 13 页区间分配 13 个单页时，每个返回值均位于范围内且互不重复，第 14 次失败。
10. 非地址顺序释放这 13 页后计数恢复为 13，且不会跨越保留边界页合并。
11. 测试结束后恢复真实各阶链表、块计数和空闲页总数，并归还完整测试区。

### [SPECIFICATION] 行为规格

#### 隔离测试环境

**Pre-Condition：** Buddy 已管理足够的真实空闲页，可以分配一个 16 页对齐块。

**Post-Condition：** 真实空闲链表在测试期间被暂存；隔离区初始只包含一个 order-4 块；测试结束时隔离链表为空，真实状态恢复，16 页测试块重新归还。

#### 分配与拆分测试

**Case 1——精确阶：** 1、2、4、8 页实际块大小与目标 order 一致。

**Case 2——向上取整：** 3 页请求得到 4 页块，5 页请求得到 8 页块，计数按实际大小减少。

**Case 3——耗尽：** 隔离区空闲页为 0 时，额外申请返回 `NULL` 且计数保持 0。

#### 伙伴与合并测试

**Case 1——真正伙伴：** 同阶、互为 XOR 伙伴且均空闲时递归合并。

**Case 2——相邻非伙伴：** 地址连续但 XOR 伙伴分别仍处于已分配状态时，两个空闲块必须保持独立。

**Case 3——不同释放顺序：** 释放顺序不影响最终恢复完整高阶块。

#### 不规则初始区测试

**Pre-Condition：** 16 页 arena 的第 0、14、15 页保持 Reserved，只登记中间 13 页。

**Post-Condition：** 所有生成块均满足 2 的幂大小、物理页号对齐和范围约束；分配结果不包含三个边界保留页；全部释放后计数恰为 13。

**Requirements：** 所有检查使用断言表达可验证条件；没有运行的结果不得写成通过；拆分和合并不直接修改总空闲页数；任何失败路径不得破坏真实分配器状态。

### 结果与后续调整

已将隔离测试区从 8 页扩展为 16 页，并补充精确阶、3/5 页向上取整、两块不重叠、耗尽失败、相邻非伙伴保持分离、乱序释放后的递归合并，以及起点偏移 1 页、长度 13 页的初始区分解和边界分配测试。测试结束前会再次分配完隔离区，确认各阶链表为空，再恢复真实链表和计数并归还 16 页块。

更新后的源码已同步到实验虚拟机，并执行：

```text
cd /home/s1nk/OperatingSystems/lab2/lab2_new
rm -f obj/kern/mm/buddy_pmm.o
make obj/kern/mm/buddy_pmm.o
```

编译输出为 `+ cc kern/mm/buddy_pmm.c`，说明新增边界测试在工程现有编译选项下通过了独立编译。目前尚未将 `pmm.c` 的当前管理器切换为 Buddy，且完整工程仍受未完成的 `page_init()` 与 Best Fit 模块影响，因此本记录只确认源码及测试代码编译成功，不宣称 `buddy_check()` 已在内核启动过程中运行通过。待公共初始化和管理器选择接口完成后，还需启动内核并以 `buddy_check()` 无断言失败及后续内核输出正常作为运行级验收结果。

<a id="myx-p9"></a>

## MYX-P9：完成个人负责内容并进行 Buddy 内核运行验证

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 在小组成员已经补全并上传 `page_init()`、Best Fit 和启动分页代码后，继续完成Lab2 内容：把 Buddy 以独立测试配置接入真实内核，运行并核验此前编写的边界测试，保证默认 Best Fit 配置和原评分不受影响；补齐 First Fit、物理页管理框架、Buddy、TLB 查询与命中/未命中、测试结果

**操作要求：** 拉取后的公共代码作为当前依赖，不改写 First Fit/Best Fit 原检查；最终普通构建仍选择 Best Fit，Buddy 只能由显式测试配置启用。所有成功结论必须来自真实编译或运行，不把独立编译写成内核运行通过。

**输出要求：** 提供可重复的 Buddy 内核测试入口、清晰的分项边界输出、默认评分回归结果、完整个人报告、TLB 源码位置与现场截图指南，并记录真实问题和修复过程。

### [RELY] 上下文信息

- `pmm.c` 已完成 DTB 内存范围读取、`Page` 数组、保留区和初始空闲区登记，默认选择 `best_fit_pmm_manager`。
- `buddy_pmm.c/.h` 已实现按阶链表、初始区分解、向上取整、分配拆分、释放合并、计数与隔离边界测试。
- Makefile 会自动编译 `kern/mm` 下的 Buddy 文件；可通过额外编译宏选择管理器，但默认评分必须保持 Best Fit。
- 工程自带 `make grade` 在临时副本注入 Page Init 和 Best Fit 测试，预期总分 20/20；该评分不覆盖 Buddy。
- 实验虚拟机实际使用 QEMU 4.1.1，源码位于 `/home/s1nk/qemu-4.1.1`，带符号程序位于 `riscv64-softmmu/qemu-system-riscv64`。
- TLB 关键实现位于 `accel/tcg/cputlb.c`、`include/exec/cpu-all.h`、`include/exec/cpu_ldst.h` 和 `target/riscv/cpu_helper.c`。

### [GUARANTEE] 交付清单

- Makefile 新增默认关闭的 `BUDDY=1` 配置；未指定时仍使用 Best Fit。
- `pmm.c` 仅在 `CONFIG_BUDDY_PMM` 定义时选择 `buddy_pmm_manager`。
- Buddy 内核运行必须显示管理器名称、实际空闲页数、各测试组 PASS 和 `check_alloc_page() succeeded!`，且不得出现 panic/assertion failure。
- Buddy 验证后重新运行原始 `make grade`，确认 Page Init、Best Fit 均为 OK，得分保持 20/20。

### [SPECIFICATION] 行为规格

#### Buddy 测试配置

**Pre-Condition：** 最新公共 `page_init()` 能向当前管理器登记真实空闲区，Buddy 文件可以参与完整内核链接。

**Post-Condition：** `make BUDDY=1` 生成选择 Buddy 的内核；普通 `make` 和 `make grade` 仍选择 Best Fit。

**Case 1——Buddy 运行：** QEMU 启动后输出 `memory management: buddy_pmm_manager`，登记 31929 个空闲页，六组边界结果全部 PASS，最后总体检查成功。

**Case 2——默认回归：** 不设置 `BUDDY` 时，评分仍检查 Page Init 和 Best Fit，并保持 20/20。

### 结果与后续调整

公共更新后先运行原始 `make grade`，Page Init 与 Best Fit 均为 OK，总分 20/20。随后在 Makefile 中增加 `BUDDY=1` 条件宏，并在 `pmm.c` 中使用 `CONFIG_BUDDY_PMM` 显式选择 Buddy；默认分支仍为 Best Fit。

第一次 Buddy 内核运行已经成功，输出管理器 `buddy_pmm_manager`、真实空闲区 31929 页以及 `check_alloc_page() succeeded!`。为了让报告截图能对应具体边界行为，又给 `buddy_check()` 增加分项 PASS 输出。新增输出第一次编译时因缺少 `<stdio.h>` 被 `-Werror` 以 `cprintf` 隐式声明拦截；补齐头文件后重新构建、运行，超量失败、拆分合并、向上取整与耗尽、相邻非伙伴、13 页不规则初始区和状态恢复六组结果全部 PASS。QEMU 最后的 signal 15 是 `timeout 10s` 主动结束既有无限循环，不是内核故障。

TLB 部分已核对 QEMU 4.1.1 的查询、victim TLB、未命中、页表翻译和填充源码路径。

最终代码已同步到 SSH 开发目录 `/home/s1nk/OperatingSystems/lab2/lab2_new`

<a id="myx-p10"></a>

## MYX-P10：整理并提交个人代码、报告、截图与 AI 交流记录

**负责人：2410933 马禹翔**

### [PROMPT] 任务说明

**任务：** 将本人负责的 Lab2 成果提交到小组仓库的 `lab2` 分支，包括 Buddy 代码及其独立测试入口、个人实验报告、六张运行/调试截图，以及已经改名为 `马禹翔_prompt.md` 的 AI 交流记录。

**操作要求：** 提交前核对实际分支和工作区差异，只暂存本人负责的文件；不得把其他成员文件、临时构建产物或无关的未跟踪目录一并提交。个人报告中的 AI 记录链接必须与改名后的文件一致。提交前需要重新运行默认评分和 Buddy 内核测试，验证通过后再提交并推送。

**输出要求：** 给出提交分支、提交范围、验证结果、提交标识和推送结果；若工作区存在未纳入提交的内容，应明确说明。

### [RELY] 上下文信息

- 小组仓库位于 `/home/s1nk/OSLab62`，目标分支为 `lab2`。
- 最新开发代码位于 `/home/s1nk/OperatingSystems/lab2/lab2_new`，其 `Makefile`、`buddy_pmm.c/.h` 和 `pmm.c` 与仓库工作区对应文件校验和一致。
- 个人报告位于 `report/2410933_马禹翔.md`，六张截图位于 `report/images/myx/`。
- AI 交流记录已经改名为 `马禹翔_prompt.md`，提交位置为 `report/马禹翔_prompt.md`。
- 仓库中存在与本人任务无关的未跟踪目录 `report--/`，不属于本次提交范围。

### [GUARANTEE] 交付清单

- 只暂存本人代码改动、个人报告、`report/images/myx/` 下六张截图和 `report/马禹翔_prompt.md`。
- 保持默认 Best Fit 行为不变；Buddy 只在显式设置 `BUDDY=1` 时启用。
- 不提交 `obj/`、`bin/`、`report--/` 等构建或无关材料。
- 提交前检查暂存文件清单和差异摘要，确认没有混入其他成员内容。
- 推送目标只能是远端 `lab2` 分支。

### [SPECIFICATION] 行为规格

#### 文件与分支检查

**Pre-Condition：** 当前 Git 分支为 `lab2`，开发目录和仓库代码一致，个人报告与截图均已完成。

**Post-Condition：** 报告链接指向同目录的 `马禹翔_prompt.md`；六张截图路径与报告引用一致；无关目录保持未跟踪且未暂存。

#### 回归与 Buddy 验证

**Case 1——默认评分：** 清理构建产物后运行 `make grade`，Page Init 和 Best Fit 均为 `OK`，总分为 `20/20`。

**Case 2——Buddy 内核运行：** 清理后运行 `timeout 15s make BUDDY=1 qemu`，输出管理器名称、31929 个空闲页、六组 Buddy `PASS` 和 `check_alloc_page() succeeded!`。最后的 signal 15 只能来自 timeout 主动结束既有内核循环，不能存在 panic 或断言失败。

#### Git 提交与推送

**Pre-Condition：** 两组验证通过，暂存区只包含本人的目标文件。

**Post-Condition：** 生成一个位于 `lab2` 分支的提交并推送到远端 `lab2`；推送后用本地与远端分支标识核对结果。

### 结果与后续调整

已在待提交仓库中重新执行默认评分，Page Init 和 Best Fit 均为 `OK`，总分保持 `20/20`。随后以 `BUDDY=1` 启动内核，确认 `buddy_pmm_manager` 管理 31929 个空闲页，六组边界结果全部 `PASS`，并输出 `check_alloc_page() succeeded!`；QEMU 最后由 15 秒 timeout 正常结束，未出现 panic 或断言失败。提交时排除无关的 `report--/` 目录和所有构建产物，只纳入本人负责的代码、报告、六张截图与本文件。

---

**2410936 林子媛：** LZY-P1、LZY-P2 保留实际使用的两份最终提示词；LZY-P3～P5 依据讨论过的学习问题整理成四段式规范稿，不作为历史聊天的逐字转录。

<a id="lzy-p1"></a>

## LZY-P1：SLUB 实现

**负责人：2410936 林子媛**

### [PROMPT] 任务说明

在 `/Users/akobayashi/OSLab/Lab2` 中设计并实现简化 SLUB，体现“底层按页分配、上层按对象分配”的两层结构。先说明接口和内存布局，再实现并接入内核。新增实现注明 `lab2 2410936`。

### [RELY] 上下文信息

阅读现有 `pmm_manager`、`alloc_pages()`、`free_pages()`、`page2pa()` 及 First Fit、Best Fit 的实现。

当前默认使用 Best Fit，本版不要求接入 Buddy，不改变现有页分配器及默认配置。

`struct Page *` 是页描述符，不是页面数据地址。`KADDR` 当前被注释，请集中封装基于 `page2pa()` 和 `va_pa_offset` 的地址转换，并检查范围。

### [GUARANTEE] 交付清单

- 设计说明：接口、大小分类、页面布局、分配释放和回收流程。
- 新增 `slub.c/.h`，放在 `/Users/akobayashi/OSLab/Lab2/code/kern/mm/`。
- 在 `pmm_init()` 完成后调用 `slub_init()`，提供独立测试入口。
- 给出真实编译结果、最小运行验证和后续测试方案；完整测试由第二份 prompt 生成。
- 不修改原有 `basic_check()`、`best_fit_check()`、评分代码、页表或 MMU 逻辑；不提交或推送 Git。

### [SPECIFICATION] 行为规格

1. 提供 `slub_init()`、`kmalloc(size_t)`、`kfree(void *)`，按单执行流实现。
2. 小对象采用 `16、32、64、128、256、512、1024、2048` 字节分类，选择最小合适类别，返回地址至少按 16 字节对齐。
3. 每个 slab 占一页；页内保存管理信息及对象槽位。正确计算元数据开销、对齐和容量，支持空闲对象复用、在用数量统计及重复释放检查。
4. 大于 2048 字节的请求走连续页路径，将块头和对齐空间计入大小；记录原始页面和申请页数，释放时完整归还。
5. slab 最后一个对象释放后立即归还页面，不保留空 slab。释放全部对象后，底层空闲页数应恢复。
6. `kmalloc(0)` 返回 `NULL`，`kfree(NULL)` 无操作；内存不足或计算溢出返回 `NULL`，不破坏已有状态。
7. 释放前校验归属、对象起始位置和分配状态；可检测的非法或重复释放应在修改状态前 `panic()`。说明无法识别所有地址复用后的悬空指针。
8. 测试方案覆盖大小边界、多页对象、对齐、混合分配、数据不串改、复用、页面回收、内存不足和异常输入。不得把未执行的测试写成已通过。

### 结果与后续调整

新增 `slub.c/.h` 并在 PMM 检查后初始化，八档小对象与连续页大对象路径已接入；默认 Best Fit 下运行。初次实现提交尚无非法释放独立测试，后由 LZY-P2 补齐，不将初次冒烟通过扩大为完整验证。Buddy 的独立配置与后端限制见 LZY-P4 和报告 4.7。


<a id="lzy-p2"></a>

## LZY-P2：SLUB 独立测试

**负责人：2410936 林子媛**

### [PROMPT] 任务说明

在 `/Users/akobayashi/OSLab/Lab2` 中，根据实际 SLUB 实现完善并执行独立测试。先列出“已有覆盖、缺失覆盖、预期结果”，再补充测试并运行，不修改分配算法或评分代码来获得通过。

### [RELY] 上下文信息

阅读现有 SLUB 实现及测试，使用 `kmalloc()`、`kfree()`、`slub_get_stats()` 和 `nr_free_pages()` 验证。

默认使用 Best Fit，不启用 Buddy。当前已存在冒烟、大小边界、多 slab 和真实内存不足测试，应保留并复用，注明原有与新增部分。

### [GUARANTEE] 交付清单

- 独立测试代码和可复现运行命令。
- 测试覆盖表：操作、预期、实际结果及有效性说明。
- 每个测试的完整日志和最终汇总，区分通过、失败与未执行。
- 不提交或推送 Git；若发现实现错误，先报告原因，不降低测试标准。

### [SPECIFICATION] 行为规格

1. 覆盖最小对象、各大小分类边界及两侧、大对象和跨多个 slab 的对象集合。
2. 混合分配并写入不同数据，验证对齐、不重叠、不串改、释放后复用及不同释放顺序。
3. 用实际页数统计验证申请和回收；全部释放后恢复本轮基线，不能写死初始空闲页数。
4. 验证真实内存不足、超大请求、整数溢出、`kmalloc(0)` 和 `kfree(NULL)`，失败不得破坏已有对象。
5. 补充对象内部指针、非所属指针、小对象重复释放、页面回收后再次释放，以及大对象非法/重复释放测试。
6. 每个预期触发 `panic()` 的用例独立启动 QEMU。只有命中该用例预期的保护信息才判通过；超时、其他崩溃或正常返回均不能算通过。
7. 不修改 `panic()` 来跳过保护；不声称能够识别地址复用后的所有悬空指针。
8. 切换编译配置时使用干净构建，或为各配置使用独立构建目录。

### 结果与后续调整

复用原有冒烟、边界、多 slab 与真实 OOM 检查，增加分类容量、大对象含头临界点、混合释放顺序和九个独立负向用例。实际汇总为 10 PASS、0 FAIL、0 NOT_RUN；负向用例须在 ARMED 后命中预期保护信息。原算法与评分未为通过而修改，完整证据见报告 5.4。


<a id="lzy-p3"></a>

## LZY-P3：理解页、描述符与 SLUB 层次

**负责人：2410936 林子媛**

### [PROMPT] 任务说明

结合 Lab2 手册和本仓库解释“页是一段内存、一个结构，还是一种机制”，说明 SLUB 在启动后的哪个阶段发挥作用。先澄清概念，再沿实际调用顺序说明。

### [RELY] 上下文信息

已有 First Fit、Best Fit 等页分配器，将在 `pmm_init()` 后初始化 SLUB。手册把连续字节合并翻译称为页，容易与 `Page`、页表、slab 混淆。`alloc_pages()` 返回描述符，不是对象数据地址。

### [GUARANTEE] 交付清单

- 分别解释物理页帧、虚拟页、`Page`、PTE、slab。
- 给出启动分页、PMM、`kmalloc/kfree` 的依赖链。
- 用一个小对象和一个多页大对象解释两层分配。

### [SPECIFICATION] 行为规格

1. 明确 4 KiB 页是地址空间单位，`Page` 是元数据，不将二者混为一谈。
2. 区分页已映射、页已分配、对象槽已分配三种状态。
3. 区分 SLUB 初始化、实际对象申请和测试入口。
4. 结合实际根项说明 1 GiB 启动大页与 4 KiB 分配单位不矛盾，不修改代码。

### 结果与后续调整

形成 4.1 的整体逻辑，理解每次分配对象不需要修改页表。

<a id="lzy-p4"></a>

## LZY-P4：分析 Buddy 与 SLUB 的契约

**负责人：2410936 林子媛**

### [PROMPT] 任务说明

分析 `slub: requires the First-Fit or Best-Fit page allocator` 报错，判断属于初始化故障、接口限制还是算法错误，给出不破坏现有检查的集成方案。

### [RELY] 上下文信息

SLUB 记录 `alloc_pages(n)` 的页数，并以 `free_pages(base, n)` 回收；当前 Buddy 向上取整到 2 的幂。默认 SLUB 规格只要求 Best Fit，组员还要独立验证 Buddy。应阅读公共接口、具体实现和 `init.c`，不能只按函数名称判断兼容。

### [GUARANTEE] 交付清单

- 解释请求与实际块长，用 3 页请求举例。
- 区分独立验证配置和真正的适配层。
- 说明必须保留的保护、统计语义与回归检查。

### [SPECIFICATION] 行为规格

1. 不删除后端保护掩盖问题，不修改 `basic_check()`、`best_fit_check()`。
2. 默认 Best Fit 执行完整 SLUB 测试；Buddy 独立配置不无条件初始化 SLUB。
3. 未实现适配层时明确支持范围，不宣称联合验证已通过。
4. 适配建议须说明实际块长获取、完整回收和统计验证。

### 结果与后续调整

统一签名不自动保证可替换性，最终保持独立配置边界，不降低测试标准。

<a id="lzy-p5"></a>

## LZY-P5：对比 Bare/Sv39 和软件 TLB

**负责人：2410936 林子媛**

### [PROMPT] 任务说明

用 GDB 比较 Bare 与 Sv39 的实际访存，进入 QEMU 源码观察翻译和软 TLB 填充，解释软件模拟与真实硬件的逻辑差异。

### [RELY] 上下文信息

内核运行于 RV64 QEMU `virt`，启动根项为 `0x200000cf`。本机是 Apple Silicon；客体使用 RISC-V GDB，宿主调试器要匹配实际运行 QEMU 的架构。调试版 4.1.1 与日常版需区分，内核 ELF 和镜像必须配对。

### [GUARANTEE] 交付清单

- 说明两个 GDB 的目标、版本和执行顺序。
- 定位 `get_physical_address()`、`tlb_set_page_with_attrs()`，给出条件断点和观察字段。
- 按输入地址、satp/特权级、PTE、物理地址、软件缓存组织实际截图与结论。

### [SPECIFICATION] 行为规格

1. 用真实访存或已知必经地址命中断点，不把检查性读内存冒充客体 load。
2. 未命中时先排查首次访问和缓存命中，不立即判定翻译未执行。
3. 按实际 PTE 区分 Sv39 最大三级深度与根级叶项提前结束。
4. 检查 Bare 是否仍建立软缓存，解释 addend 与宿主 RAM 定位。
5. 保留真实环境限制，不声称测量实机 TLB，不覆盖系统 QEMU。

### 结果与后续调整

形成 5.5 的六步证据链，解释为何独立编译、选择首次 BSS 写入，并区分客体地址与宿主指针。
