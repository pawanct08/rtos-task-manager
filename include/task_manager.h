/**
 * @file task_manager.h
 * @brief Priority-based preemptive task manager on FreeRTOS (ARM Cortex-M4)
 *
 * Task State Machine:
 *   READY → RUNNING → BLOCKED → READY
 *                  ↘ SUSPENDED
 *
 * AUTOSAR OS mapping:
 *   ISR Cat1  → High-priority, no OS services (bare handler)
 *   ISR Cat2  → Can call OS services (FreeRTOS FromISR APIs)
 *   Tasks     → Basic/Extended tasks mapped to FreeRTOS tasks
 */

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

/* ─── Task Priorities (Rate Monotonic — shorter period ⟹ higher priority) ─── */
#define PRIORITY_HIGH   (configMAX_PRIORITIES - 1)   /* 20 ms period */
#define PRIORITY_MED    (configMAX_PRIORITIES - 2)   /* 50 ms period */
#define PRIORITY_LOW    (configMAX_PRIORITIES - 3)   /* 100 ms period */
#define PRIORITY_IDLE   tskIDLE_PRIORITY

/* ─── Stack sizes (in words) ─── */
#define STACK_HIGH      256
#define STACK_MED       256
#define STACK_LOW       256

/* ─── Maximum tasks tracked by the manager ─── */
#define MAX_MANAGED_TASKS   8

/* ─── Task State Machine ─── */
typedef enum {
    TASK_STATE_READY      = 0,
    TASK_STATE_RUNNING    = 1,
    TASK_STATE_BLOCKED    = 2,
    TASK_STATE_SUSPENDED  = 3,
    TASK_STATE_DELETED    = 4,
} TaskStateMachine_t;

/* ─── AUTOSAR OS Category ─── */
typedef enum {
    AUTOSAR_BASIC_TASK    = 0,  /* Non-preemptable, no waiting */
    AUTOSAR_EXTENDED_TASK = 1,  /* Can wait on events          */
    AUTOSAR_ISR_CAT1      = 2,  /* No OS services              */
    AUTOSAR_ISR_CAT2      = 3,  /* OS-aware ISR                */
} AutosarCategory_t;

/* ─── Task Control Block extension (sits alongside FreeRTOS TCB) ─── */
typedef struct {
    TaskHandle_t        handle;
    char                name[configMAX_TASK_NAME_LEN];
    UBaseType_t         priority;
    uint32_t            period_ms;       /* Rate-monotonic period         */
    uint32_t            deadline_ms;     /* Relative deadline             */
    uint32_t            wcet_ms;         /* Worst-case execution time     */
    uint32_t            exec_count;      /* Number of activations         */
    uint32_t            deadline_miss;   /* Deadline overrun counter      */
    TaskStateMachine_t  state;
    AutosarCategory_t   autosar_cat;
    bool                active;
} TaskInfo_t;

/* ─── Public API ─── */

/**
 * @brief Initialise the task manager (call before scheduler starts).
 */
void TaskManager_Init(void);

/**
 * @brief Register a managed task.
 * @return index in internal table, or -1 on failure.
 */
int TaskManager_Register(TaskHandle_t handle,
                         const char  *name,
                         UBaseType_t  priority,
                         uint32_t     period_ms,
                         uint32_t     deadline_ms,
                         uint32_t     wcet_ms,
                         AutosarCategory_t cat);

/**
 * @brief Refresh state machines for all registered tasks.
 *        Call periodically from a monitor task.
 */
void TaskManager_Refresh(void);

/**
 * @brief Print a formatted status table over UART.
 */
void TaskManager_PrintStatus(void);

/**
 * @brief Return pointer to internal task table (read-only).
 */
const TaskInfo_t *TaskManager_GetTable(int *count_out);

/**
 * @brief Increment exec_count for a running task (call from task body).
 */
void TaskManager_IncExec(TaskHandle_t handle);

#endif /* TASK_MANAGER_H */
