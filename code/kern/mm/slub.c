#include <best_fit_pmm.h>
#include <default_pmm.h>
#include <list.h>
#include <pmm.h>
#include <slub.h>
#include <stdio.h>
#include <string.h>

// lab2 2410936: in-page metadata, partial slabs, and immediate empty-page return.
#define SLUB_ALIGNMENT 16
#define SLUB_CLASS_COUNT 8
#define SLUB_BITMAP_WORDS ((PGSIZE / SLUB_ALIGNMENT + 63) / 64)
#define SLUB_MAGIC 0x534C55424C414232ULL

enum slub_kind { SLUB_SMALL, SLUB_LARGE };

struct slub_block {
    uint64_t magic;
    enum slub_kind kind;
    struct Page *page;
    size_t npages;
    list_entry_t active_link;
};

struct slub_slab {
    struct slub_block block;
    list_entry_t partial_link;
    size_t class_index;
    size_t capacity;
    size_t inuse;
    void *free_objects;
    uint64_t allocated[SLUB_BITMAP_WORDS];
};

struct slub_large {
    struct slub_block block;
    size_t requested_size;
};

#define SLAB_HEADER_SIZE ROUNDUP(sizeof(struct slub_slab), SLUB_ALIGNMENT)
#define LARGE_HEADER_SIZE ROUNDUP(sizeof(struct slub_large), SLUB_ALIGNMENT)

static const size_t class_sizes[SLUB_CLASS_COUNT] = {
    16, 32, 64, 128, 256, 512, 1024, 2048,
};
static list_entry_t partial_slabs[SLUB_CLASS_COUNT];
static list_entry_t active_blocks;
static const struct pmm_manager *backend;
static bool initialized;

static void
slub_require_ready(void) {
    if (!initialized || pmm_manager != backend) {
        panic("slub: not initialized or page allocator changed");
    }
}

// Page descriptors and page contents are different objects; validate before use.
static void *
slub_page_kva(struct Page *page, size_t npages) {
    uintptr_t descriptor = (uintptr_t)page;
    uintptr_t descriptors = (uintptr_t)pages;
    size_t count = npage - nbase;
    if (descriptor < descriptors ||
        (descriptor - descriptors) % sizeof(*pages) != 0) {
        panic("slub: invalid page descriptor");
    }
    size_t index = (descriptor - descriptors) / sizeof(*pages);
    if (index >= count || npages == 0 || npages > count - index) {
        panic("slub: page allocation outside managed memory");
    }
    uintptr_t pa = page2pa(page);
    if (pa < KERNEL_BEGIN_PADDR ||
        pa > (uintptr_t)-1 - va_pa_offset) {
        panic("slub: invalid physical page address");
    }
    uintptr_t kva = pa + va_pa_offset;
    if (kva < KERNBASE || kva >= KERNTOP ||
        npages > (KERNTOP - kva) / PGSIZE) {
        panic("slub: page allocation outside kernel mapping");
    }
    return (void *)kva;
}

void
slub_init(void) {
    if (initialized) {
        slub_require_ready();
        return;
    }
    if (pages == NULL || npage <= nbase ||
        npage > PHYSICAL_MEMORY_END / PGSIZE ||
        va_pa_offset != PHYSICAL_MEMORY_OFFSET) {
        panic("slub: physical memory initialization is incomplete");
    }
    if (pmm_manager != &best_fit_pmm_manager &&
        pmm_manager != &default_pmm_manager) {
        panic("slub: requires the First-Fit or Best-Fit page allocator");
    }
    assert(SLAB_HEADER_SIZE + class_sizes[SLUB_CLASS_COUNT - 1] <= PGSIZE);
    assert(LARGE_HEADER_SIZE < PGSIZE);
    backend = pmm_manager;
    list_init(&active_blocks);
    for (size_t i = 0; i < SLUB_CLASS_COUNT; i++) {
        list_init(&partial_slabs[i]);
    }
    initialized = 1;
    cprintf("slub: initialized (16-byte alignment, sizes 16..2048)\n");
}

static void
slub_register(struct slub_block *block, enum slub_kind kind,
              struct Page *page, size_t npages) {
    block->magic = SLUB_MAGIC;
    block->kind = kind;
    block->page = page;
    block->npages = npages;
    list_init(&block->active_link);
    list_add(&active_blocks, &block->active_link);
}

static struct slub_slab *
slub_new_slab(size_t class_index) {
    struct Page *page = alloc_pages(1);
    if (page == NULL) {
        return NULL;
    }
    struct slub_slab *slab = slub_page_kva(page, 1);
    memset(slab, 0, sizeof(*slab));
    slab->class_index = class_index;
    size_t object_size = class_sizes[class_index];
    slab->capacity = (PGSIZE - SLAB_HEADER_SIZE) / object_size;
    list_init(&slab->partial_link);
    uint8_t *objects = (uint8_t *)slab + SLAB_HEADER_SIZE;
    for (size_t i = slab->capacity; i > 0; i--) {
        void *object = objects + (i - 1) * object_size;
        *(void **)object = slab->free_objects;
        slab->free_objects = object;
    }
    slub_register(&slab->block, SLUB_SMALL, page, 1);
    list_add(&partial_slabs[class_index], &slab->partial_link);
    return slab;
}

static size_t
slub_slot_index(struct slub_slab *slab, void *ptr) {
    assert(slab->class_index < SLUB_CLASS_COUNT);
    uintptr_t start = (uintptr_t)slab + SLAB_HEADER_SIZE;
    uintptr_t address = (uintptr_t)ptr;
    size_t object_size = class_sizes[slab->class_index];
    if (address < start || (address - start) % object_size != 0 ||
        (address - start) / object_size >= slab->capacity) {
        panic("slub: pointer is not an object start");
    }
    return (address - start) / object_size;
}

static void *
slub_alloc_small(size_t class_index) {
    struct slub_slab *slab;
    if (list_empty(&partial_slabs[class_index])) {
        slab = slub_new_slab(class_index);
        if (slab == NULL) {
            return NULL;
        }
    } else {
        slab = to_struct(list_next(&partial_slabs[class_index]),
                         struct slub_slab, partial_link);
    }
    assert(slab->block.magic == SLUB_MAGIC && slab->class_index == class_index);
    assert(slab->inuse < slab->capacity && slab->free_objects != NULL);
    void *object = slab->free_objects;
    size_t index = slub_slot_index(slab, object);
    uint64_t mask = (uint64_t)1 << (index % 64);
    assert(!(slab->allocated[index / 64] & mask));
    slab->free_objects = *(void **)object;
    slab->allocated[index / 64] |= mask;
    slab->inuse++;
    if (slab->inuse == slab->capacity) {
        assert(slab->free_objects == NULL);
        list_del_init(&slab->partial_link);
    }
    return object;
}

static void *
slub_alloc_large(size_t size) {
    if (size > (size_t)-1 - LARGE_HEADER_SIZE) {
        return NULL;
    }
    size_t total = size + LARGE_HEADER_SIZE;
    // Division avoids the overflow in ROUNDUP(total, PGSIZE).
    size_t npages = total / PGSIZE + (total % PGSIZE != 0);
    if (npages > npage - nbase || npages > nr_free_pages()) {
        return NULL;
    }
    struct Page *page = alloc_pages(npages);
    if (page == NULL) {
        return NULL;
    }
    struct slub_large *large = slub_page_kva(page, npages);
    memset(large, 0, sizeof(*large));
    large->requested_size = size;
    slub_register(&large->block, SLUB_LARGE, page, npages);
    return (uint8_t *)large + LARGE_HEADER_SIZE;
}

void *
kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    slub_require_ready();
    for (size_t i = 0; i < SLUB_CLASS_COUNT; i++) {
        if (size <= class_sizes[i]) {
            return slub_alloc_small(i);
        }
    }
    return slub_alloc_large(size);
}

// Search trusted live headers instead of dereferencing a header derived from ptr.
static struct slub_block *
slub_find_block(uintptr_t address) {
    for (list_entry_t *le = list_next(&active_blocks); le != &active_blocks;
         le = list_next(le)) {
        struct slub_block *block = to_struct(le, struct slub_block, active_link);
        assert(block->magic == SLUB_MAGIC);
        uintptr_t start = (uintptr_t)block;
        if (address >= start && address - start < block->npages * PGSIZE) {
            return block;
        }
    }
    return NULL;
}

static void
slub_release_block(struct slub_block *block) {
    struct Page *page = block->page;
    size_t npages = block->npages;
    list_del_init(&block->active_link);
    block->magic = 0;
    free_pages(page, npages);
}

void
kfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    slub_require_ready();
    uintptr_t address = (uintptr_t)ptr;
    if (address < KERNBASE || address >= KERNTOP ||
        address % SLUB_ALIGNMENT != 0) {
        panic("slub: invalid free pointer");
    }
    struct slub_block *block = slub_find_block(address);
    if (block == NULL) {
        panic("slub: pointer is not owned by a live allocation");
    }
    if (block->kind == SLUB_LARGE) {
        if (address != (uintptr_t)block + LARGE_HEADER_SIZE) {
            panic("slub: pointer is not a large-object start");
        }
        slub_release_block(block);
        return;
    }
    assert(block->kind == SLUB_SMALL);
    struct slub_slab *slab = (struct slub_slab *)block;
    size_t index = slub_slot_index(slab, ptr);
    uint64_t mask = (uint64_t)1 << (index % 64);
    if (!(slab->allocated[index / 64] & mask)) {
        panic("slub: double free of a slab object");
    }
    assert(slab->inuse > 0 && slab->inuse <= slab->capacity);
    bool was_full = slab->inuse == slab->capacity;
    slab->allocated[index / 64] &= ~mask;
    slab->inuse--;
    if (slab->inuse == 0) {
        if (!list_empty(&slab->partial_link)) {
            list_del_init(&slab->partial_link);
        }
        slub_release_block(block);
        return;
    }
    *(void **)ptr = slab->free_objects;
    slab->free_objects = ptr;
    if (was_full) {
        list_add(&partial_slabs[slab->class_index], &slab->partial_link);
    }
}

void
slub_get_stats(struct slub_stats *stats) {
    slub_require_ready();
    assert(stats != NULL);
    memset(stats, 0, sizeof(*stats));
    for (list_entry_t *le = list_next(&active_blocks); le != &active_blocks;
         le = list_next(le)) {
        struct slub_block *block = to_struct(le, struct slub_block, active_link);
        assert(block->magic == SLUB_MAGIC);
        if (block->kind == SLUB_SMALL) {
            stats->live_objects += ((struct slub_slab *)block)->inuse;
            stats->slab_pages += block->npages;
        } else {
            assert(block->kind == SLUB_LARGE);
            stats->live_objects++;
            stats->large_pages += block->npages;
        }
    }
}
