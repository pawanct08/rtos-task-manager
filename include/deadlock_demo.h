// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 pawanct08

#ifndef DEADLOCK_DEMO_H
#define DEADLOCK_DEMO_H

/*=============================================================================
 * deadlock_demo.h — Week 3
 *
 * A controlled, repeatable deadlock scenario using MutexGuard.
 *
 * Scenario (classic dining philosophers / AB-BA pattern):
 *   Task A: takes Mutex_1, then tries Mutex_2
 *   Task B: takes Mutex_2, then tries Mutex_1
 *
 * Without MutexGuard → permanent deadlock, system hangs.
 * With MutexGuard    → DFS cycle detected before blocking,
 *                      offending task aborts, logs via UART,
 *                      system continues running.
 *
 * Run via CLI:  > deadlock
 *=============================================================================*/

/**
 * @brief Create the two deadlock demo tasks.
 *        Call once after TaskManager_Init() and MutexGuard_Init().
 */
void DeadlockDemo_Start(void);

#endif /* DEADLOCK_DEMO_H */
