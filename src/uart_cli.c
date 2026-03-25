#include "uart_cli.h"
#include "task_manager.h"
#include "mutex_guard.h"
#include "mem_pool.h"
#include "autosar_os.h"
#include "latency_profiler.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Simplified mps2-an386 UART registers (PL011 at 0x40004000) */
#define UART0_BASE 0x40004000UL
#define UART_DR    *((volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR    *((volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_IBRD  *((volatile uint32_t *)(UART0_BASE + 0x24))
#define UART_FBRD  *((volatile uint32_t *)(UART0_BASE + 0x28))
#define UART_LCRH  *((volatile uint32_t *)(UART0_BASE + 0x2C))
#define UART_CTL   *((volatile uint32_t *)(UART0_BASE + 0x30))

#define SCB_AIRCR  *((volatile uint32_t *)0xE000ED0C)

#define FR_TXFF (1 << 5)
#define FR_RXFE (1 << 4)

void UART_Init(void) {
    UART_CTL = 0;
    /* 115200 baud @ 25 MHz (mps2-an386): IBRD=13, FBRD=36 */
    UART_IBRD = 13;
    UART_FBRD = 36;
    /* 8-bit, no parity, 1-stop, FIFO enable */
    UART_LCRH = 0x70;
    /* Enable UART + TX + RX */
    UART_CTL = 0x301;
}

void UART_PutChar(char c) {
    if (c == '\n') UART_PutChar('\r');
    while (UART_FR & FR_TXFF) {}
    UART_DR = c;
}

void UART_PutStr(const char *s) {
    while (*s) UART_PutChar(*s++);
}

char UART_GetChar(void) {
    if (UART_FR & FR_RXFE) return 0;
    return (char)(UART_DR & 0xFF);
}

void UART_Printf(const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    UART_PutStr(buf);
}

/* ── CLI help text ── */
static void cli_print_help(void) {
    UART_PutStr("\r\nCommands:\r\n");
    UART_PutStr("  tasks    — task state table (priority, state, deadline, stack)\r\n");
    UART_PutStr("  mutexes  — mutex ownership + deadlock detection status\r\n");
    UART_PutStr("  pools    — memory pool utilisation\r\n");
    UART_PutStr("  autosar  — AUTOSAR OS category mapping\r\n");
    UART_PutStr("  latency  — context switch latency stats + CSV dump\r\n");
    UART_PutStr("  reset    — software reset\r\n");
    UART_PutStr("  help     — show this message\r\n");
}

/* ── AUTOSAR category table ── */
static void cli_print_autosar(void) {
    int count = 0;
    const TaskInfo_t *tbl = TaskManager_GetTable(&count);

    UART_PutStr("\r\n=== AUTOSAR OS Mapping ===\r\n");
    UART_PutStr("+----------------+----------------+--------+---------+---------+\r\n");
    UART_PutStr("| Name           | Category       | Period | Deadline| WCET    |\r\n");
    UART_PutStr("+----------------+----------------+--------+---------+---------+\r\n");

    const char *cat_names[] = {
        "BasicTask      ", "ExtendedTask   ", "ISR Category 1 ", "ISR Category 2 "
    };

    for (int i = 0; i < count; i++) {
        const TaskInfo_t *t = &tbl[i];
        if (!t->active) continue;
        const char *cat = (t->autosar_cat <= 3) ? cat_names[t->autosar_cat] : "Unknown        ";
        UART_Printf("| %-14s | %s | %5ums | %6ums | %5ums |\r\n",
                    t->name, cat,
                    (unsigned)t->period_ms,
                    (unsigned)t->deadline_ms,
                    (unsigned)t->wcet_ms);
    }
    UART_PutStr("+----------------+----------------+--------+---------+---------+\r\n\r\n");
}

/* ── dispatch ── */
static void cli_dispatch(const char *cmd) {
    if      (strcmp(cmd, "tasks")   == 0) { TaskManager_PrintStatus(); }
    else if (strcmp(cmd, "mutexes") == 0) { MutexGuard_PrintStatus(); }
    else if (strcmp(cmd, "pools")   == 0) { MemPool_DumpAll(); }
    else if (strcmp(cmd, "autosar") == 0) { cli_print_autosar(); }
    else if (strcmp(cmd, "latency") == 0) { lp_print_stats(); lp_dump_csv(); }
    else if (strcmp(cmd, "reset")   == 0) {
        UART_PutStr("Resetting...\r\n");
        vTaskDelay(pdMS_TO_TICKS(20));
        SCB_AIRCR = 0x05FA0004;
    }
    else if (cmd[0] != '\0') {
        UART_Printf("Unknown: '%s'. Type 'help'.\r\n", cmd);
    }
}

void CLI_Task(void *pvParam) {
    (void)pvParam;

    /* Small delay so other tasks print their boot messages first */
    vTaskDelay(pdMS_TO_TICKS(500));

    UART_PutStr("\r\n>> UART CLI ready. Type 'help' for commands.\r\n> ");

    char buf[32];
    int  idx = 0;

    for (;;) {
        char c = UART_GetChar();

        if (c != 0) {
            UART_PutChar(c);   /* echo */

            if (c == '\r' || c == '\n') {
                UART_PutStr("\r\n");
                buf[idx] = '\0';
                cli_dispatch(buf);
                idx = 0;
                UART_PutStr("> ");
            } else if (c == 127 || c == '\b') {
                /* Backspace */
                if (idx > 0) { idx--; UART_PutStr("\b \b"); }
            } else if (idx < (int)sizeof(buf) - 1 && c >= 32 && c <= 126) {
                buf[idx++] = c;
            }
        } else {
            /* Nothing in RX FIFO — yield so other tasks can run */
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
