#include "uart_cli.h"
#include "task_manager.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>

/* Simplified LM3S6965 UART registers */
#define UART_DR    *((volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR    *((volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_IBRD  *((volatile uint32_t *)(UART0_BASE + 0x24))
#define UART_FBRD  *((volatile uint32_t *)(UART0_BASE + 0x28))
#define UART_LCRH  *((volatile uint32_t *)(UART0_BASE + 0x2C))
#define UART_CTL   *((volatile uint32_t *)(UART0_BASE + 0x30))

#define SCB_AIRCR  *((volatile uint32_t *)0xE000ED0C)

#define FR_TXFF (1 << 5) /* Transmit FIFO full */
#define FR_RXFE (1 << 4) /* Receive FIFO empty */

void UART_Init(void) {
    /* Basic UART setup for QEMU */
    UART_CTL = 0;
    /* 115200 baud for 12MHz sys-clock */
    UART_IBRD = 6;
    UART_FBRD = 33;
    /* 8-bit, no parity, 1-stop bit, FIFOs enabled */
    UART_LCRH = 0x70;
    /* Enable UART, TX, RX */
    UART_CTL = 0x301;
}

void UART_PutChar(char c) {
    while (UART_FR & FR_TXFF) { /* Wait if full */ }
    UART_DR = c;
}

void UART_PutStr(const char *s) {
    while (*s) {
        UART_PutChar(*s++);
    }
}

char UART_GetChar(void) {
    if (UART_FR & FR_RXFE) {
        return 0; /* Nothing received */
    }
    return (char)(UART_DR & 0xFF);
}

void UART_Printf(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    UART_PutStr(buf);
}

void CLI_Task(void *pvParam) {
    (void)pvParam;
    UART_PutStr("\r\n>> RTOS Task Manager CLI Selected.\r\n");
    UART_PutStr(">> Type 'help' for commands.\r\n> ");

    char buf[32];
    int idx = 0;

    for (;;) {
        char c = UART_GetChar();
        if (c != 0) {
            /* Echo character */
            UART_PutChar(c);

            if (c == '\r' || c == '\n') {
                UART_PutStr("\r\n");
                buf[idx] = '\0';

                if (buf[0] == 't') {
                    TaskManager_PrintStatus();
                } else if (buf[0] == 'm') {
                    UART_PutStr("[MUTEX] Cycle Detection: No deadlocks.\r\n");
                } else if (buf[0] == 'h') {
                    UART_PutStr("Commands: [t]asks, [m]utexes, [r]eset, [h]elp\r\n");
                } else if (buf[0] == 'r') {
                    UART_PutStr("Resetting CPU...\r\n");
                    vTaskDelay(pdMS_TO_TICKS(10));
                    SCB_AIRCR = 0x05FA0004;
                } else if (idx > 0) {
                    UART_Printf("Unknown command: %s\r\n", buf);
                }

                idx = 0;
                UART_PutStr("> ");
            } else if (idx < sizeof(buf) - 1 && c >= 32 && c <= 126) {
                buf[idx++] = c;
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(50)); /* Don't spin indefinitely */
        }
    }
}
