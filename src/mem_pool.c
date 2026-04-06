// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 pawanct08

/**
 * @file mem_pool.c
 * @brief O(1) fixed-size block memory pool — no heap fragmentation.
 */

#include "mem_pool.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* ─── Global pool registry (for CLI) ─── */
static MemPool_t *s_pools[MAX_POOLS];
static int        s_pool_count = 0;

/* ─── Public API ─── */

void MemPool_Init(MemPool_t  *pool,
                  uint8_t    *buf,
                  size_t      block_size,
                  uint32_t    num_blocks,
                  const char *name)
{
    configASSERT(pool       != NULL);
    configASSERT(buf        != NULL);
    configASSERT(block_size >= sizeof(MemBlock_t));
    configASSERT(num_blocks  > 0);

    pool->buffer      = buf;
    pool->block_size  = block_size;
    pool->num_blocks  = num_blocks;
    pool->free_count  = num_blocks;
    pool->alloc_count = 0;
    pool->fail_count  = 0;
    strncpy(pool->name, name, sizeof(pool->name) - 1);
    pool->name[sizeof(pool->name) - 1] = '\0';

    /* Build intrusive free-list */
    pool->free_list = NULL;
    for (uint32_t i = 0; i < num_blocks; i++) {
        MemBlock_t *blk = (MemBlock_t *)(buf + i * block_size);
        blk->next       = pool->free_list;
        pool->free_list = blk;
    }
}

void *MemPool_Alloc(MemPool_t *pool)
{
    void *ptr = NULL;

    /* Enter critical section (ISR-safe) */
    UBaseType_t saved = taskENTER_CRITICAL_FROM_ISR();

    if (pool->free_list != NULL) {
        MemBlock_t *blk = pool->free_list;
        pool->free_list = blk->next;
        pool->free_count--;
        pool->alloc_count++;
        ptr = (void *)blk;
    } else {
        pool->fail_count++;
    }

    taskEXIT_CRITICAL_FROM_ISR(saved);
    return ptr;
}

void MemPool_Free(MemPool_t *pool, void *ptr)
{
    if (ptr == NULL) return;

    UBaseType_t saved = taskENTER_CRITICAL_FROM_ISR();

    MemBlock_t *blk = (MemBlock_t *)ptr;
    blk->next       = pool->free_list;
    pool->free_list  = blk;
    pool->free_count++;

    taskEXIT_CRITICAL_FROM_ISR(saved);
}

void MemPool_PrintStatus(const MemPool_t *pool)
{
    uint32_t used = pool->num_blocks - pool->free_count;
    UART_Printf("  %-12s  total=%-3u  free=%-3u  used=%-3u  alloc=%-5u  fail=%u\r\n",
                pool->name,
                (unsigned)pool->num_blocks,
                (unsigned)pool->free_count,
                (unsigned)used,
                (unsigned)pool->alloc_count,
                (unsigned)pool->fail_count);
}

void MemPool_Register(MemPool_t *pool)
{
    if (s_pool_count < MAX_POOLS) {
        s_pools[s_pool_count++] = pool;
    }
}

void MemPool_DumpAll(void)
{
    UART_Printf("\r\n=== Memory Pool Status ===\r\n");
    UART_Printf("  %-12s  %-8s  %-8s  %-8s  %-9s  %s\r\n",
                "Pool", "Total", "Free", "Used", "Allocs", "Fails");
    UART_Printf("  %-12s  %-8s  %-8s  %-8s  %-9s  %s\r\n",
                "----", "-----", "----", "----", "------", "-----");
    for (int i = 0; i < s_pool_count; i++) {
        MemPool_PrintStatus(s_pools[i]);
    }
    UART_Printf("\r\n");
}
