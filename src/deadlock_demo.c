/*=============================================================================
 * deadlock_demo.c 
 *
 * Classic AB-BA deadlock pattern — detected and prevented by MutexGuard.
 *
 * Timeline (without guard):
 *   t=0   Task A takes Mutex_1
 *   t=0   Task B takes Mutex_2
 *   t=1   Task A tries Mutex_2 → BLOCKS (B holds it)
 *   t=1   Task B tries Mutex_1 → BLOCKS (A holds it)
 *   t=∞   Both tasks blocked forever → deadlock
 *
 * Timeline (with MutexGuard DFS):
 *   t=0   Task A takes Mutex_1  → graph: A→M1 (owner)
 *   t=0   Task B takes Mutex_2  → graph: B→M2 (owner)
 *   t=1   Task A tries Mutex_2  → wait edge added: A waiting on M2
 *           DFS: M2 owner=B, B not waiting → no cycle yet → A blocks
 *   t=1   Task B tries Mutex_1  → wait edge added: B waiting on M1
 *           DFS: M1 owner=A, A waiting on M2, M2 owner=B → CYCLE!
 *           MutexGuard returns pdFALSE — B does NOT block
 *           Log: "[DEADLOCK] Task 'DemoB' would deadlock on mutex 'Mutex_1'!"
 *   t=2   Task B releases Mutex_2, Task A unblocks, system continues
 *=============================================================================*/

#include "deadlock_demo.h"
#include "mutex_guard.h"
#include "task_manager.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"

/* Shared mutex indices — created once, reused across demo runs */
static int s_mutex1 = -1;
static int s_mutex2 = -1;
static bool s_initialized = false;

/*---------------------------------------------------------------------------
 * Task A: takes M1 first, then M2 (correct order)
 *---------------------------------------------------------------------------*/
static void demo_task_a(void *pv)
{
    (void)pv;
    uint32_t round = 0;

    for (;;) {
        round++;
        UART_Printf("\r\n[DemoA] Round %lu — taking Mutex_1...\r\n",
                    (unsigned long)round);

        if (MutexGuard_Take(s_mutex1, pdMS_TO_TICKS(500)) == pdTRUE) {
            UART_PutStr("[DemoA] Got Mutex_1. Simulating work (200ms)...\r\n");
            vTaskDelay(pdMS_TO_TICKS(200));

            UART_PutStr("[DemoA] Now trying Mutex_2...\r\n");
            if (MutexGuard_Take(s_mutex2, pdMS_TO_TICKS(500)) == pdTRUE) {
                UART_PutStr("[DemoA] Got Mutex_2. Both held — doing critical work.\r\n");
                vTaskDelay(pdMS_TO_TICKS(100));
                MutexGuard_Give(s_mutex2);
                UART_PutStr("[DemoA] Released Mutex_2.\r\n");
            } else {
                UART_PutStr("[DemoA] Mutex_2 timeout or deadlock avoided.\r\n");
            }

            MutexGuard_Give(s_mutex1);
            UART_PutStr("[DemoA] Released Mutex_1.\r\n");
        } else {
            UART_PutStr("[DemoA] Mutex_1 timeout.\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*---------------------------------------------------------------------------
 * Task B: takes M2 first, then M1 (REVERSE order → AB-BA deadlock pattern)
 *---------------------------------------------------------------------------*/
static void demo_task_b(void *pv)
{
    (void)pv;
    uint32_t round = 0;

    /* Slight offset so A gets M1 before B starts */
    vTaskDelay(pdMS_TO_TICKS(100));

    for (;;) {
        round++;
        UART_Printf("\r\n[DemoB] Round %lu — taking Mutex_2...\r\n",
                    (unsigned long)round);

        if (MutexGuard_Take(s_mutex2, pdMS_TO_TICKS(500)) == pdTRUE) {
            UART_PutStr("[DemoB] Got Mutex_2. Simulating work (200ms)...\r\n");
            vTaskDelay(pdMS_TO_TICKS(200));

            /*
             * This is the dangerous moment:
             * A holds M1 and is waiting for M2 (which B holds).
             * B now tries M1 → cycle: B→M1→A→M2→B
             * MutexGuard detects this and returns pdFALSE immediately.
             */
            UART_PutStr("[DemoB] Now trying Mutex_1 (REVERSE ORDER)...\r\n");
            if (MutexGuard_Take(s_mutex1, pdMS_TO_TICKS(500)) == pdTRUE) {
                UART_PutStr("[DemoB] Got Mutex_1 (no deadlock this time).\r\n");
                vTaskDelay(pdMS_TO_TICKS(100));
                MutexGuard_Give(s_mutex1);
                UART_PutStr("[DemoB] Released Mutex_1.\r\n");
            } else {
                /*
                 * MutexGuard returned pdFALSE — deadlock was prevented.
                 * In production code you would: retry with backoff,
                 * release held resources, or escalate to error handler.
                 */
                UART_PutStr("[DemoB] *** DEADLOCK PREVENTED by MutexGuard ***\r\n");
                UART_PutStr("[DemoB] Releasing Mutex_2 and backing off.\r\n");
            }

            MutexGuard_Give(s_mutex2);
            UART_PutStr("[DemoB] Released Mutex_2.\r\n");
        } else {
            UART_PutStr("[DemoB] Mutex_2 timeout.\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*---------------------------------------------------------------------------*/
void DeadlockDemo_Start(void)
{
    if (s_initialized) {
        UART_PutStr("[DeadlockDemo] Already running.\r\n");
        return;
    }

    UART_PutStr("\r\n[DeadlockDemo] Starting AB-BA deadlock scenario...\r\n");
    UART_PutStr("[DeadlockDemo] Task A: M1 -> M2 (correct order)\r\n");
    UART_PutStr("[DeadlockDemo] Task B: M2 -> M1 (reverse order = deadlock risk)\r\n");
    UART_PutStr("[DeadlockDemo] MutexGuard will detect and prevent the cycle.\r\n\r\n");

    s_mutex1 = MutexGuard_Create("Mutex_1");
    s_mutex2 = MutexGuard_Create("Mutex_2");

    xTaskCreate(demo_task_a, "DemoA", 512, NULL, 2, NULL);
    xTaskCreate(demo_task_b, "DemoB", 512, NULL, 2, NULL);

    s_initialized = true;
}
