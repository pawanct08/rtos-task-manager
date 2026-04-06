// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 pawanct08

#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "mem_pool.h"
#include "mutex_guard.h"
#include "uart_cli.h"
#include "autosar_os.h"
#include "latency_profiler.h"
#include "deadlock_demo.h"
#include "mem_pool_stress.h"

/* Use TaskManager_GetCycles() rather than accessing DWT_CYCCNT directly —
 * the DWT is not modelled on QEMU mps2-an386 (unmapped PPB = BusFault). */

/*---------------------------------------------------------------------------
 * AUTOSAR Alarm callbacks — called by software timer on period expiry
 *---------------------------------------------------------------------------*/
static void alarm_health_monitor(void) {
    /* In a real system: check task deadlines, watchdog kick, fault logging */
}

static void alarm_diagnostics(void) {
    /* In a real system: DTC snapshot, freeze frame capture */
}

/*---------------------------------------------------------------------------
 * Core application tasks (Rate Monotonic: shorter period = higher priority)
 *---------------------------------------------------------------------------*/
static void HighTx_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL);
        for (volatile int i = 0; i < 10000; i++);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void MedCalc_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL);
        for (volatile int i = 0; i < 20000; i++);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void LowPoll_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL);
        for (volatile int i = 0; i < 5000; i++);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*---------------------------------------------------------------------------
 * Probe task — DWT scheduling jitter measurement
 *---------------------------------------------------------------------------*/
static void Probe_Task(void *pv) {
    (void)pv;
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        uint32_t t0 = TaskManager_GetCycles();
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(10));
        uint32_t elapsed  = TaskManager_GetCycles() - t0;
        uint32_t expected = (LP_CPU_CLOCK_HZ / 1000) * 10;
        uint32_t jitter   = (elapsed > expected) ? (elapsed - expected) : 0;
        lp_record(jitter);
    }
}

/*---------------------------------------------------------------------------
 * Verbose assert hook — prints file:line so silent for(;;) hangs are visible
 *---------------------------------------------------------------------------*/
void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
    UART_Printf( "\r\n!!! configASSERT FAILED: %s : %d\r\n", pcFile, (int) ulLine );
    for( ;; );
}

/*---------------------------------------------------------------------------
 * main
 *---------------------------------------------------------------------------*/
int main(void) {

    /* ── 1. Hardware + subsystem init ─────────────────────────────────────── */
    UART_Init();
    UART_PutStr("\r\n================================================\r\n");
    UART_PutStr("    RTOS Task Manager — FreeRTOS / Cortex-M4\r\n");
    UART_PutStr("    QEMU mps2-an386  |  Cortex-M4 RTOS\r\n");
    UART_PutStr("================================================\r\n\r\n");

    TaskManager_Init();   /* also initialises DWT */
    UART_PutStr("CHK1: TaskManager OK\r\n");
    MutexGuard_Init();
    UART_PutStr("CHK2: MutexGuard OK\r\n");
    AutosarOS_Init();
    UART_PutStr("CHK3: AutosarOS OK\r\n");
    lp_init();
    UART_PutStr("CHK4: lp_init OK\r\n");

    /* Heap sanity probe — verify pvPortMalloc works before first xTaskCreate */
    {
        void *pProbe = pvPortMalloc( 4U );
        if( pProbe != NULL ) { vPortFree( pProbe ); UART_PutStr( "CHK4h: heap OK\r\n" ); }
        else                 { UART_PutStr( "CHK4h: heap FAIL -- aborting\r\n" ); for(;;); }
    }

    /* ── 2. Core application tasks ────────────────────────────────────────── */
    TaskHandle_t hHigh = NULL, hMed = NULL, hLow = NULL;

    UART_PutStr("CHK4a: calling xTaskCreate(HighTx)\r\n");
    if (xTaskCreate(HighTx_Task,  "HighTx",  256, NULL, PRIORITY_HIGH, &hHigh) != pdPASS) {
        UART_PutStr("!!! xTaskCreate HighTx FAILED (heap exhausted?)\r\n");
    }
    UART_PutStr("CHK4b: xTaskCreate returned\r\n");
    TaskManager_Register(hHigh, "HighTx", PRIORITY_HIGH,
                         20, 20, 5, AUTOSAR_BASIC_TASK);
    UART_PutStr("CHK5: HighTx created\r\n");

    xTaskCreate(MedCalc_Task, "MedCalc", 256, NULL, PRIORITY_MED,  &hMed);
    TaskManager_Register(hMed, "MedCalc", PRIORITY_MED,
                         50, 50, 10, AUTOSAR_BASIC_TASK);
    UART_PutStr("CHK6: MedCalc created\r\n");

    xTaskCreate(LowPoll_Task, "LowPoll", 256, NULL, PRIORITY_LOW,  &hLow);
    TaskManager_Register(hLow, "LowPoll", PRIORITY_LOW,
                         100, 100, 15, AUTOSAR_BASIC_TASK);
    UART_PutStr("CHK7: LowPoll created\r\n");

    /* ── 3. Latency probe ────────────────────────────────────────────────── */
    xTaskCreate(Probe_Task, "Probe", 256, NULL, PRIORITY_HIGH, NULL);
    UART_PutStr("CHK8: Probe created\r\n");

    /* ── 4. Deadlock demo + memory pool stress ────────────────────────── */
    DeadlockDemo_Start();
    UART_PutStr("CHK9: DeadlockDemo started\r\n");
    MemPoolStress_Start();
    UART_PutStr("CHK10: MemPoolStress started\r\n");

    /* ── 5. AUTOSAR alarms ──────────────────────────────────────────────── */
    AutosarOS_SetRelAlarm("HealthMon",   500,  alarm_health_monitor);
    AutosarOS_SetRelAlarm("Diagnostics", 1000, alarm_diagnostics);

    /* Validate RMS schedulability for the 3 core tasks */
    RMSEntry_t rms_tasks[] = {
        { "HighTx",  5000,  20000  },   /* WCET=5ms, T=20ms  */
        { "MedCalc", 10000, 50000  },   /* WCET=10ms, T=50ms */
        { "LowPoll", 15000, 100000 },   /* WCET=15ms, T=100ms */
    };
    AutosarOS_CheckSchedulability(rms_tasks, 3);

    /* ── 6. UART CLI ──────────────────────────────────────────────────────── */
    xTaskCreate(CLI_Task, "UART_CLI",
                configMINIMAL_STACK_SIZE + 256, NULL, PRIORITY_LOW - 1, NULL);

    UART_PutStr("\r\nAll tasks created. Starting scheduler...\r\n");
    UART_PutStr("Type 'help' for commands once running.\r\n\r\n");

    vTaskStartScheduler();

    /* Should never reach here */
    for (;;);
    return 0;
}


