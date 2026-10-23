#ifdef SLUB_TEST
#include <pmm.h>
#include <slub.h>
#include <stdio.h>
#include <string.h>

// lab2 2410936: original allocation, reuse and mixed-size smoke checks.
static void
check_bytes(const void *ptr, size_t size, uint8_t value) {
    const uint8_t *bytes = ptr;
    for (size_t i = 0; i < size; i++) {
        assert(bytes[i] == value);
    }
}

// lab2 2410673: integration regression tests use public allocator interfaces.
static void
check_clean(size_t expected_pages) {
    struct slub_stats stats;
    slub_get_stats(&stats);
    assert(stats.live_objects == 0 && stats.slab_pages == 0 &&
           stats.large_pages == 0 && nr_free_pages() == expected_pages);
}

static void
check_size_boundaries(void) {
    size_t before = nr_free_pages();
    const size_t boundaries[] = {16, 32, 64, 128, 256, 512, 1024, 2048,
                                4096, 8192};
    for (size_t i = 0; i < sizeof(boundaries) / sizeof(boundaries[0]); i++) {
        for (int delta = -1; delta <= 1; delta++) {
            size_t size = boundaries[i] + delta;
            void *a = kmalloc(size), *b = kmalloc(size);
            assert(a != NULL && b != NULL);
            uintptr_t x = (uintptr_t)a, y = (uintptr_t)b;
            assert(x % 16 == 0 && y % 16 == 0);
            assert(x + size <= y || y + size <= x);
            memset(a, 0xA5, size);
            memset(b, 0x5A, size);
            check_bytes(a, size, 0xA5);
            check_bytes(b, size, 0x5A);
            kfree(a);
            check_bytes(b, size, 0x5A);
            kfree(b);
            check_clean(before);
        }
    }
    cprintf("[SLUB boundary] size-class and page boundaries: PASS\n");
}

static void
check_multi_slab(void) {
    // Static storage avoids exceeding the kernel's 8 KiB bootstrap stack.
    static void *objects[600];
    size_t before = nr_free_pages();
    for (size_t i = 0; i < 600; i++) {
        objects[i] = kmalloc(16);
        assert(objects[i] != NULL && (uintptr_t)objects[i] % 16 == 0);
        for (size_t j = 0; j < i; j++) {
            assert(objects[i] != objects[j]);
        }
        memset(objects[i], (int)(i % 251 + 1), 16);
    }
    struct slub_stats full, partial;
    slub_get_stats(&full);
    assert(full.live_objects == 600 && full.slab_pages >= 3 &&
           full.large_pages == 0);
    assert(nr_free_pages() == before - full.slab_pages);
    for (size_t i = 0; i < 600; i++) {
        check_bytes(objects[i], 16, (uint8_t)(i % 251 + 1));
    }
    for (size_t i = 0; i < 600; i += 2) {
        kfree(objects[i]);
        objects[i] = NULL;
    }
    slub_get_stats(&partial);
    assert(partial.live_objects == 300 && partial.slab_pages == full.slab_pages);
    for (size_t i = 0; i < 600; i += 2) {
        objects[i] = kmalloc(16);
        assert(objects[i] != NULL);
        for (size_t j = 0; j < 600; j++) {
            if (i != j && objects[j] != NULL) {
                assert(objects[i] != objects[j]);
            }
        }
        memset(objects[i], (int)(i % 251 + 1), 16);
    }
    slub_get_stats(&partial);
    assert(partial.live_objects == 600 && partial.slab_pages == full.slab_pages);
    for (size_t i = 600; i > 0; i--) {
        check_bytes(objects[i - 1], 16, (uint8_t)((i - 1) % 251 + 1));
        kfree(objects[i - 1]);
    }
    check_clean(before);
    cprintf("[SLUB boundary] multiple slabs, full/partial reuse and reclaim: PASS\n");
}

static void
check_out_of_memory(void) {
    size_t before = nr_free_pages();
    void *live = kmalloc(24);
    assert(live != NULL);
    memset(live, 0xA7, 24);
    list_entry_t held;
    list_init(&held);
    size_t held_count = 0;
    struct Page *page;
    while ((page = alloc_page()) != NULL) {
        list_add(&held, &page->page_link);
        held_count++;
    }
    assert(held_count == before - 1 && nr_free_pages() == 0);
    struct slub_stats initial, failed;
    slub_get_stats(&initial);
    assert(kmalloc(16) == NULL && kmalloc(2048) == NULL &&
           kmalloc(8192) == NULL && kmalloc((size_t)-1) == NULL);
    slub_get_stats(&failed);
    assert(failed.live_objects == initial.live_objects &&
           failed.slab_pages == initial.slab_pages &&
           failed.large_pages == initial.large_pages && nr_free_pages() == 0);
    check_bytes(live, 24, 0xA7);
    // Exhausting backing pages must still allow reuse of existing free slots.
    static void *slots[256];
    size_t count = 0;
    while (count < 256 && (slots[count] = kmalloc(24)) != NULL) {
        memset(slots[count], 0x39, 24);
        count++;
    }
    assert(count > 0 && count < 256 && nr_free_pages() == 0);
    for (size_t i = 0; i < count; i++) {
        check_bytes(slots[i], 24, 0x39);
        kfree(slots[i]);
    }
    check_bytes(live, 24, 0xA7);
    kfree(live);
    while (!list_empty(&held)) {
        list_entry_t *le = list_next(&held);
        page = le2page(le, page_link);
        list_del_init(le);
        free_page(page);
    }
    check_clean(before);
    cprintf("[SLUB boundary] real OOM, unchanged failure state and recovery: PASS\n");
}

void
slub_check(void) {
    size_t before = nr_free_pages();
    struct slub_stats stats;
    slub_get_stats(&stats);
    assert(stats.live_objects == 0 && stats.slab_pages == 0 &&
           stats.large_pages == 0);
    cprintf("[SLUB smoke] begin: %lu free pages\n", (unsigned long)before);

    void *a = kmalloc(24);
    void *b = kmalloc(24);
    assert(a != NULL && b != NULL && a != b);
    assert((uintptr_t)a % 16 == 0 && (uintptr_t)b % 16 == 0);
    assert(ROUNDDOWN((uintptr_t)a, PGSIZE) ==
           ROUNDDOWN((uintptr_t)b, PGSIZE));
    assert(nr_free_pages() == before - 1);
    memset(a, 0xA5, 24);
    memset(b, 0x5A, 24);
    slub_init();
    slub_get_stats(&stats);
    assert(stats.live_objects == 2 && stats.slab_pages == 1);
    kfree(a);
    void *reused = kmalloc(24);
    assert(reused == a);
    memset(reused, 0xC3, 24);
    check_bytes(b, 24, 0x5A);
    kfree(b);
    check_bytes(reused, 24, 0xC3);
    kfree(reused);
    assert(nr_free_pages() == before);
    cprintf("[SLUB smoke] reuse, live count, empty-page return: PASS\n");

    const size_t sizes[] = {1, 16, 17, 2048, 2049, 8192};
    void *objects[sizeof(sizes) / sizeof(sizes[0])];
    size_t count = sizeof(sizes) / sizeof(sizes[0]);
    for (size_t i = 0; i < count; i++) {
        objects[i] = kmalloc(sizes[i]);
        assert(objects[i] != NULL && (uintptr_t)objects[i] % 16 == 0);
        for (size_t j = 0; j < i; j++) {
            uintptr_t x = (uintptr_t)objects[i];
            uintptr_t y = (uintptr_t)objects[j];
            assert(x + sizes[i] <= y || y + sizes[j] <= x);
        }
        memset(objects[i], (int)(0x30 + i), sizes[i]);
    }
    slub_get_stats(&stats);
    assert(stats.live_objects == count && stats.slab_pages == 3 &&
           stats.large_pages == 4);
    assert(nr_free_pages() == before - 7);
    assert(kmalloc(0) == NULL && kmalloc((size_t)-1) == NULL);
    kfree(NULL);
    for (size_t i = count; i > 0; i--) {
        check_bytes(objects[i - 1], sizes[i - 1], (uint8_t)(0x30 + i - 1));
        kfree(objects[i - 1]);
    }
    slub_get_stats(&stats);
    assert(stats.live_objects == 0 && stats.slab_pages == 0 &&
           stats.large_pages == 0 && nr_free_pages() == before);
    cprintf("[SLUB smoke] mixed sizes, alignment, data, large objects: PASS\n");
    cprintf("[SLUB smoke] PASS: restored %lu free pages\n", (unsigned long)before);
    check_size_boundaries();
    check_multi_slab();
    check_out_of_memory();
    check_clean(before);
    cprintf("[SLUB boundary] all integration tests: PASS\n");
}
#endif
