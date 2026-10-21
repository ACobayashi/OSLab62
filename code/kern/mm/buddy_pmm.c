#include <buddy_pmm.h>
#include <list.h>
#include <pmm.h>

// lab2 2410933: Buddy physical page allocator.

#define BUDDY_MAX_ORDER 31

struct buddy_free_area {
    list_entry_t free_list;
    size_t nr_blocks;
};

static struct buddy_free_area buddy_free_areas[BUDDY_MAX_ORDER + 1];
static size_t buddy_nr_free;

static size_t
buddy_block_size(unsigned int order) {
    return (size_t)1 << order;
}

static unsigned int
buddy_order_for_pages(size_t n) {
    assert(n > 0);

    size_t block_size = 1;
    unsigned int order = 0;
    while (block_size < n && order < BUDDY_MAX_ORDER) {
        block_size <<= 1;
        order ++;
    }
    return (block_size < n) ? BUDDY_MAX_ORDER + 1 : order;
}

static bool
buddy_block_in_range(ppn_t ppn, size_t block_size) {
    return ppn >= nbase && ppn < npage && block_size <= npage - ppn;
}

static struct Page *
buddy_page_from_ppn(ppn_t ppn) {
    assert(ppn >= nbase && ppn < npage);
    return &pages[ppn - nbase];
}

static void
buddy_add_free_block(struct Page *base, unsigned int order) {
    assert(order <= BUDDY_MAX_ORDER);
    assert(!PageReserved(base) && !PageProperty(base));

    base->property = buddy_block_size(order);
    SetPageProperty(base);
    list_add(&buddy_free_areas[order].free_list, &base->page_link);
    buddy_free_areas[order].nr_blocks ++;
}

static void
buddy_remove_free_block(struct Page *base, unsigned int order) {
    assert(order <= BUDDY_MAX_ORDER);
    assert(PageProperty(base));
    assert(base->property == buddy_block_size(order));
    assert(buddy_free_areas[order].nr_blocks > 0);

    list_del_init(&base->page_link);
    ClearPageProperty(base);
    base->property = 0;
    buddy_free_areas[order].nr_blocks --;
}

static struct Page *
buddy_find_free_buddy(struct Page *base, unsigned int order) {
    size_t block_size = buddy_block_size(order);
    ppn_t base_ppn = page2ppn(base);
    ppn_t buddy_ppn = base_ppn ^ block_size;

    if (!buddy_block_in_range(buddy_ppn, block_size)) {
        return NULL;
    }

    struct Page *buddy = buddy_page_from_ppn(buddy_ppn);
    if (!PageProperty(buddy) || buddy->property != block_size) {
        return NULL;
    }
    return buddy;
}

static void
buddy_insert_merged_block(struct Page *base, unsigned int order) {
    while (order < BUDDY_MAX_ORDER) {
        struct Page *buddy = buddy_find_free_buddy(base, order);
        if (buddy == NULL) {
            break;
        }

        buddy_remove_free_block(buddy, order);
        if (buddy < base) {
            base = buddy;
        }
        order ++;
    }
    buddy_add_free_block(base, order);
}

static void
buddy_init(void) {
    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order ++) {
        list_init(&buddy_free_areas[order].free_list);
        buddy_free_areas[order].nr_blocks = 0;
    }
    buddy_nr_free = 0;
}

static void
buddy_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);

    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(PageReserved(p));
        p->flags = 0;
        p->property = 0;
        set_page_ref(p, 0);
        list_init(&p->page_link);
    }

    p = base;
    size_t remaining = n;
    while (remaining > 0) {
        ppn_t ppn = page2ppn(p);
        unsigned int order = 0;

        while (order < BUDDY_MAX_ORDER) {
            size_t next_size = buddy_block_size(order + 1);
            if (next_size > remaining || (ppn & (next_size - 1)) != 0) {
                break;
            }
            order ++;
        }

        size_t block_size = buddy_block_size(order);
        buddy_insert_merged_block(p, order);
        p += block_size;
        remaining -= block_size;
    }
    buddy_nr_free += n;
}

static struct Page *
buddy_alloc_pages(size_t n) {
    assert(n > 0);

    unsigned int target_order = buddy_order_for_pages(n);
    if (target_order > BUDDY_MAX_ORDER) {
        return NULL;
    }

    unsigned int order = target_order;
    while (order <= BUDDY_MAX_ORDER &&
           list_empty(&buddy_free_areas[order].free_list)) {
        order ++;
    }
    if (order > BUDDY_MAX_ORDER) {
        return NULL;
    }

    list_entry_t *le = list_next(&buddy_free_areas[order].free_list);
    struct Page *base = le2page(le, page_link);
    buddy_remove_free_block(base, order);

    while (order > target_order) {
        order --;
        size_t half_size = buddy_block_size(order);
        struct Page *right_buddy = base + half_size;
        buddy_add_free_block(right_buddy, order);
    }

    size_t allocated_size = buddy_block_size(target_order);
    assert(buddy_nr_free >= allocated_size);
    buddy_nr_free -= allocated_size;

    // Keep the actual rounded block size on the allocated head.  The
    // PG_property bit remains clear, so the block is not considered free.
    base->property = allocated_size;
    return base;
}

static void
buddy_free_pages(struct Page *base, size_t n) {
    assert(n > 0);

    unsigned int order = buddy_order_for_pages(n);
    assert(order <= BUDDY_MAX_ORDER);

    size_t block_size = buddy_block_size(order);
    ppn_t base_ppn = page2ppn(base);
    assert(buddy_block_in_range(base_ppn, block_size));
    assert((base_ppn & (block_size - 1)) == 0);
    assert(!PageReserved(base) && !PageProperty(base));
    assert(base->property == block_size);

    for (struct Page *p = base; p != base + block_size; p ++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        p->property = 0;
        set_page_ref(p, 0);
        list_init(&p->page_link);
    }

    buddy_nr_free += block_size;
    buddy_insert_merged_block(base, order);
}

static size_t
buddy_nr_free_pages(void) {
    return buddy_nr_free;
}

static bool
buddy_blocks_overlap(struct Page *left, size_t left_n,
                     struct Page *right, size_t right_n) {
    return left < right + right_n && right < left + left_n;
}

static void
buddy_check(void) {
    size_t nr_free_store = buddy_nr_free_pages();
    assert(buddy_alloc_pages(nr_free_store + 1) == NULL);

    // Remove one aligned arena from the real allocator, then temporarily
    // isolate the free lists so the split/merge expectations are deterministic.
    struct Page *arena = buddy_alloc_pages(16);
    assert(arena != NULL && arena->property == 16);
    assert(buddy_nr_free_pages() == nr_free_store - 16);

    list_entry_t free_list_store[BUDDY_MAX_ORDER + 1];
    size_t nr_blocks_store[BUDDY_MAX_ORDER + 1];
    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order ++) {
        free_list_store[order] = buddy_free_areas[order].free_list;
        nr_blocks_store[order] = buddy_free_areas[order].nr_blocks;
        list_init(&buddy_free_areas[order].free_list);
        buddy_free_areas[order].nr_blocks = 0;
    }
    buddy_nr_free = 0;

    for (struct Page *p = arena; p != arena + 16; p ++) {
        p->flags = 0;
        SetPageReserved(p);
        p->property = 0;
        set_page_ref(p, 0);
        list_init(&p->page_link);
    }
    buddy_init_memmap(arena, 16);
    assert(buddy_nr_free_pages() == 16);
    assert(buddy_free_areas[4].nr_blocks == 1);
    assert(buddy_alloc_pages(17) == NULL);

    // Splitting one larger block must produce two order-0 buddies.
    struct Page *p0 = buddy_alloc_pages(1);
    struct Page *p1 = buddy_alloc_pages(1);
    assert(p0 != NULL && p1 != NULL);
    assert(p0->property == 1 && p1->property == 1);
    assert((page2ppn(p0) ^ 1) == page2ppn(p1));
    assert(buddy_nr_free_pages() == 14);

    buddy_free_pages(p0, 1);
    buddy_free_pages(p1, 1);
    assert(buddy_nr_free_pages() == 16);

    // The two single-page buddies must be reusable as one order-1 block.
    struct Page *p2 = buddy_alloc_pages(2);
    assert(p2 == (p0 < p1 ? p0 : p1));
    assert(p2->property == 2);
    buddy_free_pages(p2, 2);
    assert(buddy_nr_free_pages() == 16);

    // Non-power-of-two requests round up to order-2 blocks and do not overlap.
    struct Page *p3 = buddy_alloc_pages(3);
    struct Page *p4 = buddy_alloc_pages(3);
    assert(p3 != NULL && p4 != NULL);
    assert(p3->property == 4 && p4->property == 4);
    assert(!buddy_blocks_overlap(p3, 4, p4, 4));
    assert(buddy_nr_free_pages() == 8);

    buddy_free_pages(p4, 3);
    buddy_free_pages(p3, 3);
    assert(buddy_nr_free_pages() == 16);

    // Two five-page requests each occupy one order-3 block.
    struct Page *p5 = buddy_alloc_pages(5);
    struct Page *p6 = buddy_alloc_pages(5);
    assert(p5 != NULL && p6 != NULL);
    assert(p5->property == 8 && p6->property == 8);
    assert(!buddy_blocks_overlap(p5, 8, p6, 8));
    assert(buddy_nr_free_pages() == 0);
    assert(buddy_alloc_pages(1) == NULL);
    buddy_free_pages(p6, 5);
    buddy_free_pages(p5, 5);
    assert(buddy_nr_free_pages() == 16);

    // Blocks [2,4) and [4,6) are adjacent but are not buddies.  Keep each
    // block's real buddy allocated and verify that these two stay separate.
    struct Page *q0 = buddy_alloc_pages(2);
    struct Page *q1 = buddy_alloc_pages(2);
    struct Page *q2 = buddy_alloc_pages(2);
    struct Page *q3 = buddy_alloc_pages(2);
    assert(q0 != NULL && q1 != NULL && q2 != NULL && q3 != NULL);
    assert(q0 + 2 == q1 && q1 + 2 == q2 && q2 + 2 == q3);

    buddy_free_pages(q1, 2);
    buddy_free_pages(q2, 2);
    assert(PageProperty(q1) && q1->property == 2);
    assert(PageProperty(q2) && q2->property == 2);
    assert(buddy_find_free_buddy(q1, 1) == NULL);
    assert(buddy_find_free_buddy(q2, 1) == NULL);

    // Freeing the real buddies in a different order must recursively rebuild
    // the complete 16-page arena.
    buddy_free_pages(q0, 2);
    buddy_free_pages(q3, 2);
    assert(buddy_nr_free_pages() == 16);
    assert(buddy_free_areas[4].nr_blocks == 1);

    // Remove the complete arena and re-register only an unaligned 13-page
    // subrange.  This checks decomposition at both alignment boundaries.
    struct Page *whole = buddy_alloc_pages(16);
    assert(whole == arena && buddy_nr_free_pages() == 0);

    for (struct Page *p = arena; p != arena + 16; p ++) {
        p->flags = 0;
        SetPageReserved(p);
        p->property = 0;
        set_page_ref(p, 0);
        list_init(&p->page_link);
    }
    buddy_init_memmap(arena + 1, 13);
    assert(buddy_nr_free_pages() == 13);
    assert(buddy_free_areas[0].nr_blocks == 1);
    assert(buddy_free_areas[1].nr_blocks == 2);
    assert(buddy_free_areas[2].nr_blocks == 2);

    struct Page *single_pages[13];
    for (size_t i = 0; i < 13; i ++) {
        single_pages[i] = buddy_alloc_pages(1);
        assert(single_pages[i] != NULL);
        assert(single_pages[i] >= arena + 1 && single_pages[i] < arena + 14);
        for (size_t j = 0; j < i; j ++) {
            assert(single_pages[i] != single_pages[j]);
        }
    }
    assert(buddy_nr_free_pages() == 0);
    assert(buddy_alloc_pages(1) == NULL);

    // Release in a non-address order and verify that the same 13 pages are
    // recovered without merging across the reserved boundary pages.
    for (size_t i = 0; i < 13; i += 2) {
        buddy_free_pages(single_pages[i], 1);
    }
    for (size_t i = 1; i < 13; i += 2) {
        buddy_free_pages(single_pages[i], 1);
    }
    assert(buddy_nr_free_pages() == 13);

    // Empty the isolated allocator again before restoring the real lists.
    for (size_t i = 0; i < 13; i ++) {
        assert(buddy_alloc_pages(1) != NULL);
    }
    assert(buddy_nr_free_pages() == 0);

    for (struct Page *p = arena; p != arena + 16; p ++) {
        p->flags = 0;
        p->property = 0;
        set_page_ref(p, 0);
        list_init(&p->page_link);
    }
    arena->property = 16;

    for (unsigned int order = 0; order <= BUDDY_MAX_ORDER; order ++) {
        buddy_free_areas[order].free_list = free_list_store[order];
        buddy_free_areas[order].nr_blocks = nr_blocks_store[order];
    }
    buddy_nr_free = nr_free_store - 16;

    buddy_free_pages(arena, 16);
    assert(buddy_nr_free_pages() == nr_free_store);
}

const struct pmm_manager buddy_pmm_manager = {
    .name = "buddy_pmm_manager",
    .init = buddy_init,
    .init_memmap = buddy_init_memmap,
    .alloc_pages = buddy_alloc_pages,
    .free_pages = buddy_free_pages,
    .nr_free_pages = buddy_nr_free_pages,
    .check = buddy_check,
};
