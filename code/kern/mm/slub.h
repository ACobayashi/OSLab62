#ifndef __KERN_MM_SLUB_H__
#define __KERN_MM_SLUB_H__

#include <defs.h>

// lab2 2410936: single-threaded object allocator above an exact-page PMM.
struct slub_stats {
    size_t live_objects;
    size_t slab_pages;
    size_t large_pages;
};

// Call after pmm_init(); repeated calls preserve existing allocations.
void slub_init(void);
// Returns 16-byte-aligned, uninitialized storage; zero/OOM/overflow returns NULL.
void *kmalloc(size_t size);
// NULL is a no-op; other arguments must be live allocation start addresses.
void kfree(void *ptr);
void slub_get_stats(struct slub_stats *stats);

#ifdef SLUB_TEST
void slub_check(void);
#endif

#endif /* !__KERN_MM_SLUB_H__ */
