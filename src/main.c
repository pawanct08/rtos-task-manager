#include "FreeRTOS.h"
#include "task.h"
#include "task_manager.h"
#include "mem_pool.h"
#include "mutex_guard.h"
#include "uart_cli.h"
#include "autosar_os.h"

int main(void) {
    /* ─── 1. Board & Hardware Init ─── */
    UART_Init();
    UART_PutStr("\r\n=================================\r\n");
    UART_PutStr("    RTOS Task Manager (QEMU)\r\n");
    UART_PutStr("=================================\r\n");

    /* ─── 2. Component Init ─── */
    TaskManager_Init();
    MutexGuard_Init();
    AutosarOS_Init();

    /* ─── 3. Verify RMS ─── */
    const RMSEntry_t tasks[] = {
        {"HighTx", 5000, 20000},   /* U = 0.25 */
        {"MedCalc", 10000, 50000}, /* U = 0.20 */
        {"LowPoll", 20000, 100000} /* U = 0.20 */
    };
    if (AutosarOS_CheckSchedulability(tasks, 3)) {
        UART_PutStr("[OK] Rate-Monotonic Schedulability Check Passed.\r\n");
    } else {
        UART_PutStr("[WARN] Schedulability overloaded!\r\n");
    }

    /* ─── 4. Create Background CLI Task ─── */
    xTaskCreate(CLI_Task, "UART_CLI", configMINIMAL_STACK_SIZE + 128, NULL, PRIORITY_LOW, NULL);

    /* ─── 5. Start Scheduler (never returns) ─── */
    UART_PutStr("Starting FreeRTOS Scheduler...\r\n");
    vTaskStartScheduler();

    /* Should not reach here */
    for (;;);
    return 0;
}
