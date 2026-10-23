#ifdef SLUB_TEST
#include <pmm.h>
#include <slub.h>
#include <stdio.h>
#include <string.h>

// lab2 2410936: a minimal smoke check, not the complete boundary test suite.
static void
check_bytes(const void *ptr, size_t size, uint8_t value) {
    const uint8_t *bytes = ptr;
    for (size_t i = 0; i < size; i++) {
        assert(bytes[i] == value);
    }
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
}
#endif
