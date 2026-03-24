/**
 * @file task_manager.c
 * @brief Task state machine, priority registration, status reporting.
 */

#include "task_manager.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* ─── Internal table ─── */
static TaskInfo_t s_table[MAX_MANAGED_TASKS];
static int        s_count = 0;
static SemaphoreHandle_t s_lock;   /* protect table from concurrent access */

/* ─── Map FreeRTOS eTaskState → our state machine ─── */
static TaskStateMachine_t map_state(eTaskState fs)
{
    switch (fs) {
        case eReady:     return TASK_STATE_READY;
        case eRunning:   return TASK_STATE_RUNNING;
        case eBlocked:   return TASK_STATE_BLOCKED;
        case eSuspended: return TASK_STATE_SUSPENDED;
        case eDeleted:   return TASK_STATE_DELETED;
        default:         return TASK_STATE_READY;
    }
}

static const char *state_name(TaskStateMachine_t s)
{
    switch (s) {
        case TASK_STATE_READY:     return "Ready    ";
        case TASK_STATE_RUNNING:   return "Running  ";
        case TASK_STATE_BLOCKED:   return "Blocked  ";
        case TASK_STATE_SUSPENDED: return "Suspended";
        case TASK_STATE_DELETED:   return "Deleted  ";
        default:                   return "Unknown  ";
    }
}

static const char *autosar_name(AutosarCategory_t c)
{
    switch (c) {
        case AUTOSAR_BASIC_TASK:    return "BasicTask  ";
        case AUTOSAR_EXTENDED_TASK: return "ExtTask    ";
        case AUTOSAR_ISR_CAT1:      return "ISR-Cat1   ";
        case AUTOSAR_ISR_CAT2:      return "ISR-Cat2   ";
        default:                    return "Unknown    ";
    }
}

/* ─── Public API ─── */

void TaskManager_Init(void)
{
    memset(s_table, 0, sizeof(s_table));
    s_count = 0;
    s_lock  = xSemaphoreCreateMutex();
    configASSERT(s_lock != NULL);
}

int TaskManager_Register(TaskHandle_t      handle,
                         const char       *name,
                         UBaseType_t       priority,
                         uint32_t          period_ms,
                         uint32_t          deadline_ms,
                         uint32_t          wcet_ms,
                         AutosarCategory_t cat)
{
    if (s_count >= MAX_MANAGED_TASKS || handle == NULL) return -1;

    xSemaphoreTake(s_lock, portMAX_DELAY);
    TaskInfo_t *t = &s_table[s_count];
    t->handle       = handle;
    strncpy(t->name, name, configMAX_TASK_NAME_LEN - 1);
    t->priority     = priority;
    t->period_ms    = period_ms;
    t->deadline_ms  = deadline_ms;
    t->wcet_ms      = wcet_ms;
    t->exec_count   = 0;
    t->deadline_miss= 0;
    t->state        = TASK_STATE_READY;
    t->autosar_cat  = cat;
    t->active       = true;
    int idx = s_count++;
    xSemaphoreGive(s_lock);
    return idx;
}

void TaskManager_Refresh(void)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (int i = 0; i < s_count; i++) {
        TaskInfo_t *t = &s_table[i];
        if (!t->active) continue;
        eTaskState fs = eTaskGetState(t->handle);
        t->state = map_state(fs);
    }
    xSemaphoreGive(s_lock);
}

void TaskManager_PrintStatus(void)
{
    TaskManager_Refresh();

    UART_Printf("\r\n=== Task Manager Status (tick=%u) ===\r\n",
                (unsigned)xTaskGetTickCount());
    UART_Printf("+----------------+----------+--+------+-----+-----+------+-------------+\r\n");
    UART_Printf("| %-14s | %-9s|Pr| Exec |Per..|Wdl..|Miss | AUTOSAR     |\r\n",
                "Name", "State");
    UART_Printf("+----------------+----------+--+------+-----+-----+------+-------------+\r\n");

    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (int i = 0; i < s_count; i++) {
        TaskInfo_t *t = &s_table[i];
        if (!t->active) continue;
        UART_Printf("| %-14s | %s|%2u| %5u|%4u |%4u |%5u | %s|\r\n",
                    t->name,
                    state_name(t->state),
                    (unsigned)t->priority,
                    (unsigned)t->exec_count,
                    (unsigned)t->period_ms,
                    (unsigned)t->deadline_ms,
                    (unsigned)t->deadline_miss,
                    autosar_name(t->autosar_cat));
    }
    xSemaphoreGive(s_lock);
    UART_Printf("+----------------+----------+--+------+-----+-----+------+-------------+\r\n\r\n");
}

const TaskInfo_t *TaskManager_GetTable(int *count_out)
{
    if (count_out) *count_out = s_count;
    return s_table;
}

void TaskManager_IncExec(TaskHandle_t handle)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (int i = 0; i < s_count; i++) {
        if (s_table[i].handle == handle) {
            s_table[i].exec_count++;
            break;
        }
    }
    xSemaphoreGive(s_lock);
}
