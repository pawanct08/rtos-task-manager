#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "mem_pool.h"
#include "mutex_guard.h"
#include "uart_cli.h"
#include "autosar_os.h"

/* ─── Demo Task Implementation ─── */

void HighTx_Task(void *pv) {
    (void)pv;
    for (;;) {
        TaskManager_IncExec(NULL); /* NULL handle uses current */
        /* Simulate some work (approx 1ms @ 50MHz) */
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

int main(void) {
    /* ─── 1. Board & Hardware Init ─── */
    UART_Init();
    UART_PutStr("\r\n=================================\r\n");
    UART_PutStr("    RTOS Task Manager (QEMU)\r\n");
    UART_PutStr("    Week 2: DWT & Profiling\r\n");
    UART_PutStr("=================================\r\n");

    /* ─── 2. Component Init ─── */
    TaskManager_Init();
    MutexGuard_Init();
    AutosarOS_Init();

    /* ─── 3. Create Managed Tasks ─── */
    TaskHandle_t hHigh, hMed, hLow;
    
    xTaskCreate(HighTx_Task, "HighTx", STACK_HIGH, NULL, PRIORITY_HIGH, &hHigh);
    TaskManager_Register(hHigh, "HighTx", PRIORITY_HIGH, 20, 20, 5, AUTOSAR_BASIC_TASK);

    xTaskCreate(MedCalc_Task, "MedCalc", STACK_MED, NULL, PRIORITY_MED, &hMed);
    TaskManager_Register(hMed, "MedCalc", PRIORITY_MED, 50, 50, 10, AUTOSAR_BASIC_TASK);

    xTaskCreate(LowPoll_Task, "LowPoll", STACK_LOW, NULL, PRIORITY_LOW, &hLow);
    TaskManager_Register(hLow, "LowPoll", PRIORITY_LOW, 100, 100, 15, AUTOSAR_BASIC_TASK);

    /* ─── 4. Create Background CLI Task ─── */
    xTaskCreate(CLI_Task, "UART_CLI", configMINIMAL_STACK_SIZE + 128, NULL, PRIORITY_LOW - 1, NULL);

    /* ─── 5. Start Scheduler ─── */
    UART_PutStr("Starting FreeRTOS Scheduler...\r\n");
    vTaskStartScheduler();

    for (;;);
    return 0;
}
