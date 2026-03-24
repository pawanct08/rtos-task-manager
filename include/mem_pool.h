/**
 * @file mem_pool.h
 * @brief Fixed-size block memory pool allocator (no heap fragmentation).
 *
 * Design:
 *   - Pool is backed by a static byte array (no malloc).
 *   - Blocks are tracked with a free-list inside each block header.
 *   - Allocation is O(1); deallocation is O(1).
 *   - Thread-safe via FreeRTOS critical section.
 *
 * Usage:
 *   MemPool_t pool;
 *   uint8_t   buf[MEM_POOL_SIZE(32, 16)];   // 16 blocks × 32 bytes
 *   MemPool_Init(&pool, buf, 32, 16);
 *   void *p = MemPool_Alloc(&pool);
 *   MemPool_Free(&pool, p);
 */

#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Macro to compute required buffer size */
#define MEM_POOL_SIZE(block_sz, num_blocks) \
    ((block_sz) * (num_blocks))

/* Maximum number of distinct pools */
#define MAX_POOLS   4

typedef struct MemBlock {
    struct MemBlock *next;   /* intrusive free-list pointer */
} MemBlock_t;

typedef struct {
    MemBlock_t  *free_list;      /* head of free block list    */
    uint8_t     *buffer;         /* backing store              */
    size_t       block_size;     /* bytes per block            */
    uint32_t     num_blocks;     /* total blocks               */
    uint32_t     free_count;     /* currently available        */
    uint32_t     alloc_count;    /* lifetime allocations       */
    uint32_t     fail_count;     /* out-of-memory count        */
    char         name[16];
} MemPool_t;

/* ─── Public API ─── */

/**
 * @brief Initialise a memory pool.
 * @param pool       Pool handle.
 * @param buf        Backing buffer (must be at least block_size * num_blocks).
 * @param block_size Bytes per block.
 * @param num_blocks Number of blocks.
 * @param name       Human-readable name for debugging.
 */
void MemPool_Init(MemPool_t  *pool,
                  uint8_t    *buf,
                  size_t      block_size,
                  uint32_t    num_blocks,
                  const char *name);

/**
 * @brief Allocate one block from the pool (O(1), ISR-safe).
 * @return Pointer to block, or NULL if pool exhausted.
 */
void *MemPool_Alloc(MemPool_t *pool);

/**
 * @brief Return a block to the pool (O(1), ISR-safe).
 * @param ptr Must have been obtained from MemPool_Alloc on the same pool.
 */
void MemPool_Free(MemPool_t *pool, void *ptr);

/**
 * @brief Print pool statistics over UART.
 */
void MemPool_PrintStatus(const MemPool_t *pool);

/**
 * @brief Register pool globally for CLI introspection.
 */
void MemPool_Register(MemPool_t *pool);

/**
 * @brief Dump all registered pools (called by CLI).
 */
void MemPool_DumpAll(void);

#endif /* MEM_POOL_H */
