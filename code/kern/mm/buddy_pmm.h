#ifndef __KERN_MM_BUDDY_PMM_H__
#define __KERN_MM_BUDDY_PMM_H__

#include <pmm.h>

// lab2 2410933: public registration object for the Buddy page allocator.
// The allocator callbacks and internal state remain private to buddy_pmm.c.
extern const struct pmm_manager buddy_pmm_manager;

#endif /* !__KERN_MM_BUDDY_PMM_H__ */
