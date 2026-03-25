#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "mem_pool.h"
#include "mutex_guard.h"
#include "uart_cli.h"
#include "autosar_os.h"
#include "latency_profiler.h"

/* DWT registers */
#define DWT_CYCCNT  (*((volatile uint32_t *)0xE0001004))

/* ─── Demo Task Implementation ─── */

void HighTx_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL);
        for (volatile int i = 0; i < 10000; i++);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void MedCalc_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL);
        for (volatile int i = 0; i < 20000; i++);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void LowPoll_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL);
        for (volatile int i = 0; i < 5000; i++);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*
 * Probe task — Week 2
 * Records DWT timestamp before blocking, reads it on wakeup.
 * Delta = scheduling latency (time from tick event to this task running).
 */
void Probe_Task(void *pv) {
    (void)pv;
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        uint32_t t0 = DWT_CYCCNT;
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(10));  /* 10ms period */
        uint32_t elapsed = DWT_CYCCNT - t0;

        /* Subtract the expected delay — remainder is scheduling jitter */
        uint32_t expected = (LP_CPU_CLOCK_HZ / 1000) * 10;
        uint32_t jitter   = (elapsed > expected) ? (elapsed - expected) : 0;
        lp_record(jitter);
    }
}

int main(void) {
    /* ─── 1. Hardware Init ─── */
    UART_Init();
    UART_PutStr("\r\n=================================\r\n");
    UART_PutStr("    RTOS Task Manager (QEMU)\r\n");
    UART_PutStr("    mps2-an386 / Cortex-M4\r\n");
    UART_PutStr("=================================\r\n");

    /* ─── 2. Component Init ─── */
    TaskManager_Init();   /* also calls TaskManager_InitDWT() */
    MutexGuard_Init();
    AutosarOS_Init();
    lp_init();

    /* ─── 3. Create Managed Tasks ─── */
    TaskHandle_t hHigh, hMed, hLow;

    xTaskCreate(HighTx_Task,  "HighTx",  STACK_HIGH, NULL, PRIORITY_HIGH,      &hHigh);
    TaskManager_Register(hHigh, "HighTx", PRIORITY_HIGH, 20, 20, 5, AUTOSAR_BASIC_TASK);

    xTaskCreate(MedCalc_Task, "MedCalc", STACK_MED,  NULL, PRIORITY_MED,       &hMed);
    TaskManager_Register(hMed,  "MedCalc", PRIORITY_MED,  50, 50, 10, AUTOSAR_BASIC_TASK);

    xTaskCreate(LowPoll_Task, "LowPoll", STACK_LOW,  NULL, PRIORITY_LOW,       &hLow);
    TaskManager_Register(hLow,  "LowPoll", PRIORITY_LOW, 100, 100, 15, AUTOSAR_BASIC_TASK);

    /* ─── 4. Probe Task (Week 2) ─── */
    xTaskCreate(Probe_Task, "Probe", 256, NULL, PRIORITY_HIGH, NULL);

    /* ─── 5. CLI Task ─── */
    xTaskCreate(CLI_Task, "UART_CLI", configMINIMAL_STACK_SIZE + 256,
                NULL, PRIORITY_LOW - 1, NULL);

    UART_PutStr("Starting FreeRTOS Scheduler...\r\n");
    UART_PutStr("Type 'help' in terminal once running.\r\n\r\n");

    vTaskStartScheduler();

    for (;;);
    return 0;
}

