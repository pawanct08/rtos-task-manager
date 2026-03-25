/*=============================================================================
 * mem_pool_stress.c — Week 3
 *
 * Concurrent memory pool stress test.
 * Verifies correctness, OOM handling, and no data corruption under load.
 *=============================================================================*/

#include "mem_pool_stress.h"
#include "mem_pool.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdint.h>

/*---------------------------------------------------------------------------
 * Pool definition — 16 blocks of 32 bytes each = 512 bytes total
 * Small enough to exhaust easily for OOM testing
 *---------------------------------------------------------------------------*/
#define STRESS_BLOCK_SIZE   32
#define STRESS_NUM_BLOCKS   16
#define FILL_PATTERN_A      0xAA
#define FILL_PATTERN_B      0xBB
#define FILL_PATTERN_C      0xCC

static MemPool_t  s_pool;
static uint8_t    s_pool_buf[MEM_POOL_SIZE(STRESS_BLOCK_SIZE, STRESS_NUM_BLOCKS)];

/* Corruption counter — incremented when pattern check fails */
static volatile uint32_t s_corruption_count = 0;
static volatile uint32_t s_verify_count     = 0;

/*---------------------------------------------------------------------------
 * Producer task — allocs a block, writes a pattern, holds briefly, frees
 *---------------------------------------------------------------------------*/
static void producer_task(void *pv)
{
    uint8_t pattern = (uint8_t)(uint32_t)pv;   /* A, B, or C */
    char name[8];
    vTaskDelay(pdMS_TO_TICKS(10 * (pattern & 0x0F)));  /* stagger start */

    for (;;) {
        void *blk = MemPool_Alloc(&s_pool);

        if (blk != NULL) {
            /* Write our pattern to every byte of the block */
            memset(blk, pattern, STRESS_BLOCK_SIZE);

            /* Simulate holding the block for some work */
            vTaskDelay(pdMS_TO_TICKS(5 + (pattern & 0x07)));

            /*
             * Verify pattern is still intact before freeing.
             * If another task corrupted this block (e.g. double-free bug),
             * the pattern check will fail.
             */
            uint8_t *b = (uint8_t *)blk;
            bool ok = true;
            for (int i = 0; i < STRESS_BLOCK_SIZE; i++) {
                if (b[i] != pattern) { ok = false; break; }
            }

            if (!ok) {
                s_corruption_count++;
                UART_Printf("[MemStress] CORRUPTION in block (pattern=0x%02X)!\r\n",
                            pattern);
            }

            MemPool_Free(&s_pool, blk);
        }
        /* No delay on OOM path — immediately retry, exercising fail_count */
        else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

/*---------------------------------------------------------------------------
 * Checker task — prints pool stats every 10s and reports corruption count
 *---------------------------------------------------------------------------*/
static void checker_task(void *pv)
{
    (void)pv;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));

        UART_PutStr("\r\n[MemStress] === Stress Report ===\r\n");
        MemPool_PrintStatus(&s_pool);
        UART_Printf("[MemStress] Corruption events : %lu\r\n",
                    (unsigned long)s_corruption_count);
        UART_Printf("[MemStress] Pattern verifies  : %lu\r\n\r\n",
                    (unsigned long)s_verify_count);

        if (s_corruption_count == 0) {
            UART_PutStr("[MemStress] Pool integrity: PASS\r\n\r\n");
        } else {
            UART_PutStr("[MemStress] Pool integrity: FAIL !!!\r\n\r\n");
        }
    }
}

/*---------------------------------------------------------------------------
 * OOM burst task — grabs ALL blocks at once to test exhaustion path,
 * then releases them. Runs every 30s so it doesn't dominate.
 *---------------------------------------------------------------------------*/
static void oom_burst_task(void *pv)
{
    (void)pv;
    void *held[STRESS_NUM_BLOCKS];

    /* Let producers warm up first */
    vTaskDelay(pdMS_TO_TICKS(15000));

    for (;;) {
        UART_PutStr("\r\n[MemStress] OOM burst: exhausting pool...\r\n");

        int grabbed = 0;
        for (int i = 0; i < STRESS_NUM_BLOCKS; i++) {
            held[i] = MemPool_Alloc(&s_pool);
            if (held[i]) grabbed++;
        }

        UART_Printf("[MemStress] Grabbed %d/%d blocks. "
                    "Producers should see OOM now.\r\n",
                    grabbed, STRESS_NUM_BLOCKS);

        /* Hold for 2 seconds — producers hit OOM path */
        vTaskDelay(pdMS_TO_TICKS(2000));

        /* Release all */
        for (int i = 0; i < grabbed; i++) {
            if (held[i]) MemPool_Free(&s_pool, held[i]);
        }

        UART_PutStr("[MemStress] Pool released. Producers resuming.\r\n");
        MemPool_PrintStatus(&s_pool);

        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

/*---------------------------------------------------------------------------*/
void MemPoolStress_Start(void)
{
    UART_PutStr("[MemStress] Initialising pool stress test...\r\n");
    UART_Printf("[MemStress] Pool: %d blocks x %d bytes = %d bytes total\r\n",
                STRESS_NUM_BLOCKS, STRESS_BLOCK_SIZE,
                STRESS_NUM_BLOCKS * STRESS_BLOCK_SIZE);

    MemPool_Init(&s_pool, s_pool_buf, STRESS_BLOCK_SIZE,
                 STRESS_NUM_BLOCKS, "StressPool");

    /* Register with CLI so 'pools' command shows it */
    MemPool_Register(&s_pool);

    /* Three producers with different patterns and timings */
    xTaskCreate(producer_task, "ProducerA", 256,
                (void *)FILL_PATTERN_A, 2, NULL);
    xTaskCreate(producer_task, "ProducerB", 256,
                (void *)FILL_PATTERN_B, 2, NULL);
    xTaskCreate(producer_task, "ProducerC", 256,
                (void *)FILL_PATTERN_C, 2, NULL);

    /* Checker + OOM burst */
    xTaskCreate(checker_task,  "PoolCheck",  512, NULL, 1, NULL);
    xTaskCreate(oom_burst_task,"OOM_Burst",  512, NULL, 1, NULL);

    UART_PutStr("[MemStress] Tasks created. Type 'pools' to monitor.\r\n\r\n");
}
