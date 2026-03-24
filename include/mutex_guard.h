/**
 * @file mutex_guard.h
 * @brief Mutex wrapper with deadlock detection via resource-allocation graph.
 *
 * Deadlock detection strategy:
 *   - Maintain a wait-for graph (tasks × mutexes).
 *   - On every mutex take, check for cycles using DFS.
 *   - If a cycle is detected, assert/log and optionally abort the offending task.
 */

#ifndef MUTEX_GUARD_H
#define MUTEX_GUARD_H

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_MUTEXES     8
#define MAX_WAITERS     MAX_MANAGED_TASKS   /* from task_manager.h */

/* Re-use task-manager limit without circular include */
#ifndef MAX_MANAGED_TASKS
#define MAX_MANAGED_TASKS 8
#endif

typedef struct {
    SemaphoreHandle_t   sem;
    char                name[16];
    TaskHandle_t        owner;       /* currently holding task, NULL if free */
    uint32_t            lock_count;  /* recursive / total lock count         */
    bool                active;
} MutexInfo_t;

/* ─── Wait-for graph entry ─── */
typedef struct {
    TaskHandle_t  waiter;  /* task waiting for the mutex */
    int           mutex_idx;
} WaitEdge_t;

/* ─── Public API ─── */

/**
 * @brief Initialise mutex guard subsystem.
 */
void MutexGuard_Init(void);

/**
 * @brief Create a tracked mutex.
 * @return index in internal table, or -1 on failure.
 */
int MutexGuard_Create(const char *name);

/**
 * @brief Acquire mutex with deadlock detection.
 * @param idx      Mutex index returned by MutexGuard_Create.
 * @param timeout  Ticks to wait (portMAX_DELAY for infinite).
 * @return pdTRUE on success, pdFALSE on timeout or deadlock.
 */
BaseType_t MutexGuard_Take(int idx, TickType_t timeout);

/**
 * @brief Release a tracked mutex.
 */
void MutexGuard_Give(int idx);

/**
 * @brief Run cycle detection on the wait-for graph.
 * @return true if a deadlock cycle is found.
 */
bool MutexGuard_DetectDeadlock(void);

/**
 * @brief Dump mutex state over UART (called by CLI).
 */
void MutexGuard_PrintStatus(void);

/**
 * @brief Get pointer to mutex table.
 */
const MutexInfo_t *MutexGuard_GetTable(int *count_out);

#endif /* MUTEX_GUARD_H */
