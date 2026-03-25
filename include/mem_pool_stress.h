#ifndef MEM_POOL_STRESS_H
#define MEM_POOL_STRESS_H

/*=============================================================================
 * mem_pool_stress.h — Week 3
 *
 * Stress tests the memory pool under concurrent task access:
 *
 *   - 3 producer tasks: alloc blocks, write pattern, hold briefly, free
 *   - 1 checker task:   verifies no block corruption (pattern check)
 *   - Exercises OOM path: intentionally exhausts pool, checks fail_count
 *   - All stats visible via CLI:  > pools
 *
 * What this proves (for interviews):
 *   - ISR-safe critical section implementation is correct under load
 *   - O(1) alloc/free holds — no slowdown at high throughput
 *   - Pool never leaks (free_count returns to num_blocks after each burst)
 *=============================================================================*/

/**
 * @brief Create stress test tasks and register the pool with CLI.
 *        Call after TaskManager_Init().
 */
void MemPoolStress_Start(void);

#endif /* MEM_POOL_STRESS_H */
