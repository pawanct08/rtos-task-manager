#include "uart_cli.h"
#include "task_manager.h"
#include "mutex_guard.h"
#include "mem_pool.h"
#include "autosar_os.h"
#include "latency_profiler.h"
#include "deadlock_demo.h"
#include "mem_pool_stress.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 * CMSDK APB UART registers (mps2-an386 UART0 at 0x40004000)
 *
 * The mps2-an386 machine uses ARM CMSDK APB UARTs, NOT PL011.
 * Register map:
 *   0x00  DATA    – TX write / RX read
 *   0x04  STATE   – bit0=TX full, bit1=RX full
 *   0x08  CTRL    – bit0=TX enable, bit1=RX enable
 *   0x10  BAUDDIV – baud-rate divisor (PCLK / baud)
 */
#define UART0_BASE   0x40004000UL
#define UART_DATA    *((volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_STATE   *((volatile uint32_t *)(UART0_BASE + 0x04))
#define UART_CTRL    *((volatile uint32_t *)(UART0_BASE + 0x08))
#define UART_BAUDDIV *((volatile uint32_t *)(UART0_BASE + 0x10))

#define STATE_TXFULL (1u << 0)   /* TX register full – wait before writing */
#define STATE_RXFULL (1u << 1)   /* RX register full – data available       */
#define CTRL_TXEN    (1u << 0)   /* Transmit enable                         */
#define CTRL_RXEN    (1u << 1)   /* Receive enable                          */

#define SCB_AIRCR  *((volatile uint32_t *)0xE000ED0C)

void UART_Init(void) {
    /* PCLK = 25 MHz on mps2-an386; 25 000 000 / 115200 ≈ 217          */
    /* QEMU does not simulate timing, but the divisor must be non-zero. */
    UART_BAUDDIV = 217;
    /* Enable TX and RX – this is the bit the old PL011 driver missed! */
    UART_CTRL = CTRL_TXEN | CTRL_RXEN;
}

void UART_PutChar(char c) {
    if (c == '\n') UART_PutChar('\r');
    while (UART_STATE & STATE_TXFULL) {}   /* wait if TX register full */
    UART_DATA = c;
}

void UART_PutStr(const char *s) {
    while (*s) UART_PutChar(*s++);
}

char UART_GetChar(void) {
    if (!(UART_STATE & STATE_RXFULL)) return 0;
    return (char)(UART_DATA & 0xFF);
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
    UART_PutStr("  autosar  — AUTOSAR OS category mapping + RMS check\r\n");
    UART_PutStr("  latency  — context switch latency stats + CSV dump\r\n");
    UART_PutStr("  deadlock — trigger AB-BA deadlock demo (MutexGuard prevents it)\r\n");
    UART_PutStr("  reset    — software reset\r\n");
    UART_PutStr("  help     — show this message\r\n");
}

/* ── dispatch ── */
static void cli_dispatch(const char *cmd) {
    if      (strcmp(cmd, "tasks")    == 0) { TaskManager_PrintStatus(); }
    else if (strcmp(cmd, "mutexes")  == 0) { MutexGuard_PrintStatus(); }
    else if (strcmp(cmd, "pools")    == 0) { MemPool_DumpAll(); }
    else if (strcmp(cmd, "autosar")  == 0) { AutosarOS_PrintMapping(); }
    else if (strcmp(cmd, "latency")  == 0) { lp_print_stats(); lp_dump_csv(); }
    else if (strcmp(cmd, "deadlock") == 0) { DeadlockDemo_Start(); }
    else if (strcmp(cmd, "help")     == 0) { cli_print_help(); }
    else if (strcmp(cmd, "reset")    == 0) {
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
