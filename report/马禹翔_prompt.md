# Lab2 AI 交流记录

成员：2410933 马禹翔

> 本文件从重新开始后的第一条正式实验问题起记录。仅记录实际发生的交流、分析和验证；不把未运行的测试写成已通过。

## 记录 1：`pmm_init()`、`page_init()` 与 `pmm_manager` 的调用关系

### [PROMPT]

**任务：** 根据当前 Lab2 源码，解释内核进入 C 入口后物理内存管理的初始化顺序，判断“内核先调用 `pmm_manager`，再调用 `pmm_init()`”是否正确，并说明 `pmm_init()` 与 `page_init()` 的包含及调用关系。

**操作要求：** 只进行源码分析和概念澄清，不修改内核源代码；区分管理框架入口、具体分配器初始化、物理内存发现与空闲区登记。

**输出要求：** 给出真实调用链，并解释每个阶段建立了什么状态、为什么顺序不能颠倒。

### [RELY]

- `kern/init/init.c` 中 `kern_init()` 调用 `pmm_init()`。
- `kern/mm/pmm.c` 中 `pmm_init()` 依次调用 `init_pmm_manager()`、`page_init()` 和 `check_alloc_page()`。
- `init_pmm_manager()` 先令全局指针 `pmm_manager` 指向具体管理器，再调用 `pmm_manager->init()`。
- `page_init()` 负责发现可管理物理内存、建立 `Page` 描述符，并通过 `init_memmap()` 调用 `pmm_manager->init_memmap(base, n)` 登记初始空闲页。

### [GUARANTEE]

- 明确外层入口是 `pmm_init()`，不是 `pmm_manager` 主动调用 `pmm_init()`。
- 区分 `pmm_manager->init()` 与 `page_init()` 的职责。
- 说明 `page_init()` 是 `pmm_init()` 的内部步骤。
- 给出从 `kern_init()` 到具体分配器的完整调用链。

### [SPECIFICATION]

**Pre-Condition：** 内核已经建立启动页表、进入 `kern_init()` 并完成此前的基本初始化，但物理页分配器尚未初始化。

**Post-Condition：** `pmm_init()` 完成后，当前分配器已选定，其内部空闲数据结构已初始化，可用物理页已登记，并已运行分配器检查。

**Case 1——选择并初始化算法：** `init_pmm_manager()` 设置 `pmm_manager` 指针并调用具体算法的 `init()`，此时只初始化链表、计数器等算法内部状态，尚未加入真实空闲页。

**Case 2——发现并登记内存：** `page_init()` 根据物理内存布局建立页描述符、排除已占用区域，并将剩余完整空闲页通过 `pmm_manager->init_memmap()` 交给当前算法。

**Requirements：** 不把 `page_init()` 与 `pmm_manager->init()` 混为一谈；不把函数指针表描述成会主动启动的对象。

### 结果与理解修正

原先“先调用 `pmm_manager` 启动内存分配模块，再调用 `pmm_init()`”的顺序需要修正。真实顺序是 `kern_init()` 先调用总入口 `pmm_init()`；`pmm_init()` 内部先选择具体的 `pmm_manager` 并初始化算法内部状态，再调用 `page_init()` 发现和登记真实可用物理页，最后运行检查。`page_init()` 是 `pmm_init()` 的组成步骤，而不是与它并列或先于它执行的外部入口。

## 记录 2：物理页、`struct Page`、`flags` 与 `property` 的关系

### [PROMPT]

**任务：** 澄清 `Page` 是否就是物理页、`flags` 是否是物理页描述符，并解释为什么 `property` 只在空闲连续块的块头有效。

**操作要求：** 依据当前工程中的 `struct Page`、页面状态宏和 First Fit 实现进行概念分析，不修改源码。

**输出要求：** 区分真实物理页、页描述符、描述符字段和空闲块元数据，并通过连续空闲页示例说明 `property` 的含义。

### [RELY]

- `kern/mm/memlayout.h` 定义 `struct Page { int ref; uint64_t flags; unsigned int property; list_entry_t page_link; }`。
- 每个 `struct Page` 描述一个物理页；真实物理页是物理内存中的一段固定大小空间。
- `flags` 通过 `PG_reserved`、`PG_property` 等位表示页面状态。
- First Fit 只把空闲连续块的首个 `Page` 挂入 `free_list`，并在该块头的 `property` 中保存块包含的页数。
- 只有 `PageProperty(page)` 为真时，当前实现才把该页的 `property` 解释为空闲块长度。

### [GUARANTEE]

- 明确 `Page` 在源码语境中通常指 `struct Page` 描述符，而不是真实 4 KiB 内存内容。
- 明确 `flags` 只是描述符的一个字段，不是完整描述符。
- 解释块头、内部页和链表节点之间的关系。
- 说明非块头页面的 `property` 可能为零或残留旧值，但没有 `PG_property` 时不应使用。

### [SPECIFICATION]

**Pre-Condition：** 已知物理内存按页划分，内核使用 `pages[]` 数组为每个可管理物理页保存一个 `struct Page`。

**Post-Condition：** 能够根据 `struct Page *`、`flags` 和 `property` 判断某个描述符是否为空闲块头，以及该块包含多少页。

**Case 1——空闲块头：** `PG_property` 置位，`property` 表示从该页开始的连续空闲页数，`page_link` 可挂入空闲链表。

**Case 2——空闲块内部页：** 不设置 `PG_property`，不单独挂入链表，`property` 不具有块长度语义。

**Case 3——已分配或保留页：** 不应按空闲块头解释；是否保留由 `PG_reserved` 等状态位判断。

**Requirements：** 判断 `property` 是否有效时必须同时检查块头标志，不能仅凭数值非零判断。

### 结果与理解修正

`struct Page` 是物理页的元数据描述符，`flags` 只是其中记录状态的一部分。真实物理页保存实际字节，描述符保存在独立的 `pages[]` 数组中。连续空闲块在链表中只需要一个节点，因此只在首个描述符上设置 `PG_property` 并用 `property` 记录整块页数；内部页不独立代表空闲块，其 `property` 即使存在旧值也应被忽略。

## 记录 3：`default_init_memmap()` 是否在探测占用页

### [PROMPT]

**任务：** 解释 `default_init_memmap()` 开头遍历 `[base, base+n)` 的循环是否用于探测已占用物理页，并判断把每页 `property` 清零是否表示后续不存在连续空闲块。

**操作要求：** 结合 `page_init()` 与 `init_memmap()` 的职责边界，以及循环之后建立块头的代码进行分析；不修改源码。

**输出要求：** 说明 `assert(PageReserved(p))` 的准确含义、清零各字段的目的，以及循环结束后连续空闲块如何建立。

### [RELY]

- `page_init()` 先完成物理内存范围判断、排除实际占用区域和页面对齐，再把已经确认可用的 `[base, base+n)` 传给 `init_memmap()`。
- `init_memmap()` 转发到当前管理器的 `default_init_memmap(base, n)`。
- `default_init_memmap()` 进入循环时要求每个描述符仍带有 `PG_reserved`，表示这些页尚未交给分配器管理。
- 循环之后执行 `base->property = n`、`SetPageProperty(base)`、`nr_free += n`，并把 `base->page_link` 插入空闲链表。

### [GUARANTEE]

- 明确该循环不是物理内存探测过程。
- 区分“实际被内核占用”与“描述符暂时标为 Reserved、尚未交给分配器”。
- 解释清零 `flags`、`property` 和引用计数是在初始化已确认可用的页。
- 说明整段连续空闲块在循环后由 `base` 统一表示。

### [SPECIFICATION]

**Pre-Condition：** `[base, base+n)` 已由上层确认是可交给分配器的连续完整物理页；其描述符在全局初始化阶段暂时统一标记为 Reserved。

**Post-Condition：** 区间中每页不再保留旧状态和引用计数；`base` 成为大小为 `n` 的唯一空闲块头，整段被登记到空闲链表，空闲页总数增加 `n`。

**Case 1——可用页区间：** 断言成立，逐页清理描述符，随后以 `base` 建立连续空闲块。

**Case 2——仍包含不应释放的页：** 若某个描述符不满足预期状态，断言用于暴露上层内存布局或重复初始化错误，而不是把该页识别后跳过。

**Requirements：** 不把循环中的临时 `property=0` 当作最终状态；必须连同循环后的块头设置一起解释。

### 结果与理解修正

该循环是在初始化一段已经确认可用的物理页，而不是探测哪些页被占用。所有页先统一清除旧标志、旧块长度和引用计数；随后只在 `base` 上设置 `property=n` 和 `PG_property`，把整个 `[base, base+n)` 表示为一个连续空闲块。真正被 OpenSBI、内核或描述符数组占用的页不会被传入该函数，并继续保持保留状态。

## 记录 4：`default_init_memmap()` 如何把空闲块插入链表

### [PROMPT]

**任务：** 解释 `default_init_memmap()` 中根据 `list_empty()`、`base < page` 和链表尾部条件执行 `list_add()` 或 `list_add_before()` 的代码，判断其是否在把空闲页连接到链表。

**操作要求：** 结合 `free_list` 的哨兵头、`base->page_link`、`le2page()` 和地址有序要求进行分析，不修改源码。

**输出要求：** 说明空链表、插入中间、追加尾部三种情况，并明确链表节点对应单页还是空闲块头。

### [RELY]

- 在执行插入代码前，`base->property=n` 且 `PG_property` 已置位，因此 `base` 表示整个 `[base, base+n)` 连续空闲块。
- `free_list` 是无数据的循环双向链表哨兵头。
- `base->page_link` 是该空闲块头嵌入的链表节点。
- `le2page(le, page_link)` 从已有链表节点取得对应空闲块头的 `struct Page *`。
- `Page` 描述符位于连续的 `pages[]` 数组中，所以比较同一数组中的 `Page *` 可以反映页描述符及对应物理页的先后顺序。

### [GUARANTEE]

- 明确只把块头挂入链表，不把块内每个页都挂入链表。
- 解释空链表时将节点加入哨兵头之后。
- 解释遇到更大地址块时插到它之前。
- 解释遍历到最后一个块时追加到尾部。
- 说明保持地址有序对释放时合并前后相邻块的作用。

### [SPECIFICATION]

**Pre-Condition：** `base` 已被初始化为大小为 `n` 的有效空闲块头，且该块尚未在 `free_list` 中。

**Post-Condition：** `base->page_link` 恰好一次地进入 `free_list`，链表仍按块头地址递增排列。

**Case 1——空链表：** 将 `base->page_link` 插入哨兵头之后，此节点同时是首节点和尾节点。

**Case 2——应插入已有块之前：** 遍历得到已有块头 `page`，若 `base < page`，则在当前节点之前插入并结束搜索。

**Case 3——地址最大：** 若已经到达最后一个已有节点仍未找到更大块，则把新节点插到最后一个节点之后。

**Requirements：** 链表节点表示连续空闲块，而非单独空闲页；代码依赖调用者保证新区间合法且不与现有空闲块重叠。

### 结果与理解修正

该代码确实把空闲内存登记到空闲链表，但登记单位是空闲连续块。整个 `[base, base+n)` 只使用 `base->page_link` 一个节点，并根据 `base` 与已有块头的地址比较插入到正确位置。地址有序使 `default_free_pages()` 在释放块时只需检查链表前驱和后继，就能判断是否存在可合并的相邻空闲块。

## 记录 5：Buddy System 的原理、实现与存在意义

### [PROMPT]

**任务：** 解释 Buddy System 是什么、如何完成初始化、分配、拆分、释放与合并，并分析在已有 First Fit（default）和 Best Fit 的情况下为什么仍需要这种分配方式。

**操作要求：** 结合当前 ucore 的页级 `pmm_manager` 接口说明，不直接修改源码；同时给出 Buddy 的优势、代价和适用场景，避免把它描述为绝对优于其他算法。

**输出要求：** 通过按阶空闲链表和具体页数示例说明算法流程，并给出在本项目中实现 Buddy 模块时需要的数据结构、函数和关键不变量。

### [RELY]

- 当前 `pmm_manager` 提供 `init`、`init_memmap`、`alloc_pages`、`free_pages`、`nr_free_pages` 和 `check` 接口。
- First Fit 和 Best Fit 使用连续空闲块；First Fit 选择第一个足够大的块，Best Fit 选择最小的足够大块。
- Buddy 要求每个受管理块的大小为 `2^order` 页，并按 order 分别组织空闲块。
- 对非 2 次幂请求，可以事先约定向上取整到最小可容纳的 2 次幂块。
- 两个块只有在大小相同、阶相同、互为伙伴且均空闲时才能合并。

### [GUARANTEE]

- 解释 order、按阶空闲链表和伙伴关系。
- 给出分配时向上取整、向高阶查找和逐级拆分的过程。
- 给出释放时伙伴定位和递归合并的过程。
- 分析 Buddy 相比 First Fit/Best Fit 在查找、合并和碎片方面的取舍。
- 列出本项目可能新增的 `buddy_pmm.h`、`buddy_pmm.c` 和核心接口。
- 说明 Buddy 不能取代对象级任意大小分配器，也不一定适合所有负载。

### [SPECIFICATION]

**Pre-Condition：** 物理内存已经按页划分，`init_memmap(base,n)` 提供一段已确认可用的连续页区间。

**Post-Condition：** 初始可用区被分解成若干对齐、不越界的 2 次幂块；分配和释放保持各阶链表、块头元数据及空闲页总数一致。

**Case 1——精确阶命中：** 目标阶链表非空时直接取出一个块。

**Case 2——需要拆分：** 目标阶为空时从更高阶取块，重复分成两个低一阶伙伴，保留一半继续拆分并把另一半放入对应阶链表。

**Case 3——释放并合并：** 根据块头和 order 找到伙伴；伙伴同阶且空闲时移除伙伴、形成更高阶块并继续，否则把当前块加入本阶链表。

**Case 4——非 2 次幂请求：** 按预先约定向上取整，计数和释放均使用实际块大小，避免语义不一致。

**Requirements：** 拆分和合并本身不改变空闲总页数；只有真正分配和释放改变计数。伙伴计算必须基于统一页索引，并检查管理范围和对齐边界。

### 结果与理解修正

Buddy 将空闲内存按 `1、2、4、8……` 页的阶组织，通过逐级二分获得所需块，通过伙伴关系快速判断能否合并。它相较单链表上的 First Fit/Best Fit，能够提供规则且通常更快的查找、拆分和合并，并抑制难以合并的外部碎片；代价是非 2 次幂请求需要向上取整，可能产生内部碎片，还需要维护多条空闲链表和阶信息。因此 Buddy 是另一种时间、空间和实现复杂度取舍，而不是对现有两种算法的无条件替代。

## 记录 6：设计 `buddy_pmm.h` 公共接口

### [PROMPT]

**任务：** 新建 `kern/mm/buddy_pmm.h`，为后续 Buddy 物理页分配模块定义公共接口。该模块后续需要支持初始化、初始内存登记、页面分配、页面释放、空闲页计数和正确性检查，并能够通过现有 `struct pmm_manager` 接入内核。

**操作要求：** 必须修改项目中的真实文件；遵循 `default_pmm.h` 与 `best_fit_pmm.h` 的封装方式；保留 `lab2` 标记并注明实现者学号 `2410933`。头文件只暴露其他模块真正需要使用的公共符号，不公开 Buddy 的各阶链表、计数器、管理范围或内部辅助函数。

**输出要求：** 创建带有独立 include guard 的 `buddy_pmm.h`，包含 `pmm.h`，并导出 Buddy 管理器注册对象。初始化、初始内存登记、分配、释放、计数、检查函数及内部基础变量在后续 `buddy_pmm.c` 中以 `static` 形式实现，通过注册对象的函数指针提供服务。

### [RELY]

- `pmm.h` 已定义 `struct pmm_manager`，其中包含 `init`、`init_memmap`、`alloc_pages`、`free_pages`、`nr_free_pages` 和 `check` 函数指针。
- `default_pmm.h` 只包含 `pmm.h` 并导出 `extern const struct pmm_manager default_pmm_manager`。
- `best_fit_pmm.h` 采用相同方式导出 `best_fit_pmm_manager`。
- Buddy 的实现细节只由 `buddy_pmm.c` 使用，不应成为内核公共 API。

### [GUARANTEE]

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

### [SPECIFICATION]

#### `buddy_pmm.h`

**Pre-Condition：** 工程已有 `pmm.h`，其他管理器头文件均通过导出一个 `const struct pmm_manager` 注册对象接入。

**Post-Condition：** 任何需要选择 Buddy 分配器的源文件均可通过包含 `buddy_pmm.h` 获取 `buddy_pmm_manager` 声明；重复包含头文件不会产生重复定义；头文件自身不分配存储，也不暴露内部可变状态。

**Case 1——正常包含：** include guard 生效，`pmm.h` 提供完整的 `struct pmm_manager` 类型，外部源文件可以引用 `buddy_pmm_manager`。

**Case 2——重复包含：** 同一翻译单元多次包含时，只保留一次有效声明。

**Requirements：** 注册对象使用 `extern const` 声明；具体定义只能在一个 `.c` 文件中出现；内部回调和变量保持模块私有；不得修改现有 First Fit、Best Fit 头文件或测试函数。

### 结果与后续调整

已创建 `kern/mm/buddy_pmm.h`，使用独立 include guard，包含 `pmm.h`，并公开 `extern const struct pmm_manager buddy_pmm_manager`。原始需求中“在头文件声明初始化、登记、分配、释放、计数函数和基础变量”的范围已按现有工程封装方式收紧：这些内容属于 Buddy 内部实现，将在 `buddy_pmm.c` 中声明为 `static`，公共头文件只暴露统一管理器注册对象。

## 记录 7：实现 `buddy_pmm.c` 完整管理模块

### [PROMPT]

**任务：** 新建 `kern/mm/buddy_pmm.c`，从零实现符合现有 ucore `struct pmm_manager` 接口的 Buddy 物理页分配模块，包括内部状态初始化、初始空闲区登记、按阶分配与拆分、释放与递归伙伴合并、空闲页计数、基础正确性检查和管理器注册。

**操作要求：** 必须修改项目中的真实文件；保留现有 First Fit、Best Fit 和测试代码；不在本步骤修改 `pmm.c` 的默认管理器选择。新增代码保留 `lab2` 标记并注明实现者学号 `2410933`。内部函数和变量必须使用 `static`，只通过 `buddy_pmm_manager` 对外提供统一接口。

**输出要求：** 生成可独立编译的 `buddy_pmm.c`，与已创建的 `buddy_pmm.h` 配套；实现完成后在实验虚拟机中编译，记录真实编译结果和需要修正的问题。

### [RELY]

- `pmm.h` 提供 `struct pmm_manager`、`pages`、`npage`、`nbase`、`page2ppn()`、`Page` 状态宏和页分配通用接口。
- `memlayout.h` 定义 `struct Page`；本模块使用 `property` 保存块的实际页数，使用 `PG_property` 表示空闲块头。
- `list.h` 提供循环双向链表操作和 `le2page()`。
- `buddy_pmm.h` 已声明 `extern const struct pmm_manager buddy_pmm_manager`。
- 可用物理页以 `init_memmap(base,n)` 形式交给模块，进入函数时页面描述符仍为 Reserved。
- 非 2 次幂请求采用向上取整到最小 `2^order` 页块的规则，释放和计数使用实际块大小。

### [GUARANTEE]

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

### [SPECIFICATION]

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

## 记录 8：第二份 Prompt——生成并补全 Buddy 边界测试

### [PROMPT]

**任务：** 针对当前 `kern/mm/buddy_pmm.c` 已实现的 Buddy 分配器，审查并补全 `buddy_check()` 测试。测试必须直接使用真实内部接口和状态约定，验证按阶分配、拆分、伙伴判断、递归合并、非 2 次幂向上取整、内存不足、区间不重叠、非伙伴相邻块以及不规则初始内存区登记。

**操作要求：** 必须修改项目中的真实 `buddy_pmm.c`；不得修改 First Fit、Best Fit 的原有检查函数，不得伪造运行输出。测试应先从真实 Buddy 管理器中取得一个对齐测试区，暂存并隔离真实各阶链表，在受控环境中运行，最后清空测试区、恢复原管理状态并归还测试页，保证测试前后总空闲页数一致。

**输出要求：** 扩展 `buddy_check()`，为每类行为设置明确断言；代码需在现有 `-Werror` 编译选项下通过。若完整工程被其他未完成模块阻断，应单独编译 `buddy_pmm.o`，并如实记录阻断原因与 Buddy 自身编译结果。

### [RELY]

- 当前实现使用 `buddy_free_areas[order]` 保存各阶空闲链表及块数，使用 `buddy_nr_free` 保存空闲页总数。
- `buddy_alloc_pages(n)` 将非 2 次幂请求向上取整，并在返回块头的 `property` 中保存实际块大小；返回块不设置 `PG_property`。
- `buddy_free_pages(base,n)` 使用相同取整规则，校验实际块大小，并通过 XOR 查找同阶伙伴。
- `buddy_find_free_buddy()` 只有在伙伴处于范围内、带有 `PG_property` 且 `property` 等于当前块大小时才返回伙伴。
- 测试可以使用模块内部 `static` 函数和变量，但不得将其暴露到公共头文件。

### [GUARANTEE]

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

### [SPECIFICATION]

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

## 记录 9：完成个人负责内容并进行 Buddy 内核运行验证

### [PROMPT]

**任务：** 在小组成员已经补全并上传 `page_init()`、Best Fit 和启动分页代码后，继续完成Lab2 内容：把 Buddy 以独立测试配置接入真实内核，运行并核验此前编写的边界测试，保证默认 Best Fit 配置和原评分不受影响；补齐 First Fit、物理页管理框架、Buddy、TLB 查询与命中/未命中、测试结果

**操作要求：** 拉取后的公共代码作为当前依赖，不改写 First Fit/Best Fit 原检查；最终普通构建仍选择 Best Fit，Buddy 只能由显式测试配置启用。所有成功结论必须来自真实编译或运行，不把独立编译写成内核运行通过。

**输出要求：** 提供可重复的 Buddy 内核测试入口、清晰的分项边界输出、默认评分回归结果、完整个人报告、TLB 源码位置与现场截图指南，并记录真实问题和修复过程。

### [RELY]

- `pmm.c` 已完成 DTB 内存范围读取、`Page` 数组、保留区和初始空闲区登记，默认选择 `best_fit_pmm_manager`。
- `buddy_pmm.c/.h` 已实现按阶链表、初始区分解、向上取整、分配拆分、释放合并、计数与隔离边界测试。
- Makefile 会自动编译 `kern/mm` 下的 Buddy 文件；可通过额外编译宏选择管理器，但默认评分必须保持 Best Fit。
- 工程自带 `make grade` 在临时副本注入 Page Init 和 Best Fit 测试，预期总分 20/20；该评分不覆盖 Buddy。
- 实验虚拟机实际使用 QEMU 4.1.1，源码位于 `/home/s1nk/qemu-4.1.1`，带符号程序位于 `riscv64-softmmu/qemu-system-riscv64`。
- TLB 关键实现位于 `accel/tcg/cputlb.c`、`include/exec/cpu-all.h`、`include/exec/cpu_ldst.h` 和 `target/riscv/cpu_helper.c`。

### [GUARANTEE]

- Makefile 新增默认关闭的 `BUDDY=1` 配置；未指定时仍使用 Best Fit。
- `pmm.c` 仅在 `CONFIG_BUDDY_PMM` 定义时选择 `buddy_pmm_manager`。
- Buddy 内核运行必须显示管理器名称、实际空闲页数、各测试组 PASS 和 `check_alloc_page() succeeded!`，且不得出现 panic/assertion failure。
- Buddy 验证后重新运行原始 `make grade`，确认 Page Init、Best Fit 均为 OK，得分保持 20/20。

### [SPECIFICATION]

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

## 记录 10：整理并提交个人代码、报告、截图与 AI 交流记录

### [PROMPT]

**任务：** 将本人负责的 Lab2 成果提交到小组仓库的 `lab2` 分支，包括 Buddy 代码及其独立测试入口、个人实验报告、六张运行/调试截图，以及已经改名为 `马禹翔_prompt.md` 的 AI 交流记录。

**操作要求：** 提交前核对实际分支和工作区差异，只暂存本人负责的文件；不得把其他成员文件、临时构建产物或无关的未跟踪目录一并提交。个人报告中的 AI 记录链接必须与改名后的文件一致。提交前需要重新运行默认评分和 Buddy 内核测试，验证通过后再提交并推送。

**输出要求：** 给出提交分支、提交范围、验证结果、提交标识和推送结果；若工作区存在未纳入提交的内容，应明确说明。

### [RELY]

- 小组仓库位于 `/home/s1nk/OSLab62`，目标分支为 `lab2`。
- 最新开发代码位于 `/home/s1nk/OperatingSystems/lab2/lab2_new`，其 `Makefile`、`buddy_pmm.c/.h` 和 `pmm.c` 与仓库工作区对应文件校验和一致。
- 个人报告位于 `report/2410933_马禹翔.md`，六张截图位于 `report/images/myx/`。
- AI 交流记录已经改名为 `马禹翔_prompt.md`，提交位置为 `report/马禹翔_prompt.md`。
- 仓库中存在与本人任务无关的未跟踪目录 `report--/`，不属于本次提交范围。

### [GUARANTEE]

- 只暂存本人代码改动、个人报告、`report/images/myx/` 下六张截图和 `report/马禹翔_prompt.md`。
- 保持默认 Best Fit 行为不变；Buddy 只在显式设置 `BUDDY=1` 时启用。
- 不提交 `obj/`、`bin/`、`report--/` 等构建或无关材料。
- 提交前检查暂存文件清单和差异摘要，确认没有混入其他成员内容。
- 推送目标只能是远端 `lab2` 分支。

### [SPECIFICATION]

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
