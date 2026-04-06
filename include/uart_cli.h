// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 pawanct08

/**
 * @file uart_cli.h
 * @brief Lightweight UART command-line interface for runtime task inspection.
 *
 * Commands:
 *   tasks      — print task status table
 *   mutexes    — print mutex / deadlock status
 *   pools      — print memory pool statistics
 *   autosar    — print AUTOSAR OS category mapping
 *   help       — list all commands
 *   reset      — software reset (SCB->AIRCR)
 */

#ifndef UART_CLI_H
#define UART_CLI_H

#include <stdint.h>
#include <stddef.h>

/* ─── Public API ─── */

/**
 * @brief Initialise UART0 at 115200-8N1 (assumes 12 MHz sys-clock).
 */
void UART_Init(void);

/**
 * @brief Transmit a single character (blocking).
 */
void UART_PutChar(char c);

/**
 * @brief Transmit a null-terminated string.
 */
void UART_PutStr(const char *s);

/**
 * @brief Formatted print over UART (subset: %s %d %u %x %c %%).
 */
void UART_Printf(const char *fmt, ...);

/**
 * @brief Non-blocking receive (returns 0 if no data).
 */
char UART_GetChar(void);

/**
 * @brief FreeRTOS task: reads UART input, processes CLI commands.
 *        Create with TaskManager at PRIORITY_LOW.
 */
void CLI_Task(void *pvParam);

#endif /* UART_CLI_H */
