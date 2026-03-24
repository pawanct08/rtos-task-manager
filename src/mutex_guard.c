/**
 * @file mutex_guard.c
 * @brief Mutex wrapper with wait-for graph deadlock detection (DFS cycle check).
 */

#include "mutex_guard.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* ─── Internal state ─── */
static MutexInfo_t s_mutexes[MAX_MUTEXES];
static int         s_mutex_count = 0;

/*
 * Wait-for graph:
 *   s_wait[i] = { waiter task, mutex_idx it is waiting on }
 *   A task waiting on mutex m → owner of m creates directed edge.
 *   Cycle = deadlock.
 */
static WaitEdge_t s_wait[MAX_MANAGED_TASKS];
static int        s_wait_count = 0;

static SemaphoreHandle_t s_guard;   /* protect the graph itself */

/* ─── DFS cycle detection ─── */
/* visited / in-stack flags indexed by mutex_idx */
static bool s_visited[MAX_MUTEXES];
static bool s_in_stack[MAX_MUTEXES];

static bool dfs(int mutex_idx)
{
    if (s_in_stack[mutex_idx]) return true;   /* back-edge → cycle */
    if (s_visited[mutex_idx])  return false;

    s_visited[mutex_idx]  = true;
    s_in_stack[mutex_idx] = true;

    /* Find the owner of this mutex and check what mutex that task is waiting on */
    TaskHandle_t owner = s_mutexes[mutex_idx].owner;
    if (owner != NULL) {
        for (int i = 0; i < s_wait_count; i++) {
            if (s_wait[i].waiter == owner) {
                if (dfs(s_wait[i].mutex_idx)) return true;
            }
        }
    }

    s_in_stack[mutex_idx] = false;
    return false;
}

/* ─── Public API ─── */

void MutexGuard_Init(void)
{
    memset(s_mutexes, 0, sizeof(s_mutexes));
    memset(s_wait,    0, sizeof(s_wait));
    s_mutex_count = 0;
    s_wait_count  = 0;
    s_guard = xSemaphoreCreateMutex();
    configASSERT(s_guard != NULL);
}

int MutexGuard_Create(const char *name)
{
    if (s_mutex_count >= MAX_MUTEXES) return -1;

    int idx = s_mutex_count++;
    MutexInfo_t *m = &s_mutexes[idx];
    m->sem    = xSemaphoreCreateMutex();
    configASSERT(m->sem != NULL);
    strncpy(m->name, name, sizeof(m->name) - 1);
    m->owner      = NULL;
    m->lock_count = 0;
    m->active     = true;
    return idx;
}

BaseType_t MutexGuard_Take(int idx, TickType_t timeout)
{
    if (idx < 0 || idx >= s_mutex_count) return pdFALSE;
    MutexInfo_t *m = &s_mutexes[idx];

    /* Record wait edge BEFORE blocking */
    xSemaphoreTake(s_guard, portMAX_DELAY);
    TaskHandle_t self = xTaskGetCurrentTaskHandle();

    /* Add waiter edge */
    if (s_wait_count < MAX_MANAGED_TASKS) {
        s_wait[s_wait_count].waiter    = self;
        s_wait[s_wait_count].mutex_idx = idx;
        s_wait_count++;
    }

    /* Deadlock check */
    memset(s_visited,  0, sizeof(s_visited));
    memset(s_in_stack, 0, sizeof(s_in_stack));
    bool deadlock = false;
    for (int i = 0; i < s_mutex_count; i++) {
        if (!s_visited[i]) {
            if (dfs(i)) { deadlock = true; break; }
        }
    }

    if (deadlock) {
        /* Remove the edge we just added (avoid poisoning the graph) */
        if (s_wait_count > 0) s_wait_count--;
        xSemaphoreGive(s_guard);
        UART_Printf("[DEADLOCK] Task '%s' would deadlock on mutex '%s'!\r\n",
                    pcTaskGetName(self), m->name);
        return pdFALSE;
    }
    xSemaphoreGive(s_guard);

    /* Now actually block on the semaphore */
    BaseType_t result = xSemaphoreTake(m->sem, timeout);

    /* Remove wait edge */
    xSemaphoreTake(s_guard, portMAX_DELAY);
    for (int i = 0; i < s_wait_count; i++) {
        if (s_wait[i].waiter == self && s_wait[i].mutex_idx == idx) {
            s_wait[i] = s_wait[--s_wait_count];
            break;
        }
    }

    if (result == pdTRUE) {
        m->owner = self;
        m->lock_count++;
    }
    xSemaphoreGive(s_guard);
    return result;
}

void MutexGuard_Give(int idx)
{
    if (idx < 0 || idx >= s_mutex_count) return;
    MutexInfo_t *m = &s_mutexes[idx];

    xSemaphoreTake(s_guard, portMAX_DELAY);
    m->owner = NULL;
    xSemaphoreGive(s_guard);

    xSemaphoreGive(m->sem);
}

bool MutexGuard_DetectDeadlock(void)
{
    xSemaphoreTake(s_guard, portMAX_DELAY);
    memset(s_visited,  0, sizeof(s_visited));
    memset(s_in_stack, 0, sizeof(s_in_stack));
    bool found = false;
    for (int i = 0; i < s_mutex_count; i++) {
        if (!s_visited[i]) {
            if (dfs(i)) { found = true; break; }
        }
    }
    xSemaphoreGive(s_guard);
    return found;
}

void MutexGuard_PrintStatus(void)
{
    xSemaphoreTake(s_guard, portMAX_DELAY);
    UART_Printf("\r\n=== Mutex Status ===\r\n");
    UART_Printf("+----------------+--------+--------------------+-------+\r\n");
    UART_Printf("| %-14s | %-6s | %-18s | Locks |\r\n", "Name", "State", "Owner");
    UART_Printf("+----------------+--------+--------------------+-------+\r\n");
    for (int i = 0; i < s_mutex_count; i++) {
        MutexInfo_t *m = &s_mutexes[i];
        const char  *owner = m->owner ? pcTaskGetName(m->owner) : "(free)";
        const char  *state = m->owner ? "LOCKED" : "FREE  ";
        UART_Printf("| %-14s | %s | %-18s | %5u |\r\n",
                    m->name, state, owner, (unsigned)m->lock_count);
    }
    UART_Printf("+----------------+--------+--------------------+-------+\r\n");

    bool dl = false;
    memset(s_visited,  0, sizeof(s_visited));
    memset(s_in_stack, 0, sizeof(s_in_stack));
    for (int i = 0; i < s_mutex_count && !dl; i++) {
        if (!s_visited[i]) dl = dfs(i);
    }
    UART_Printf("Deadlock detected: %s\r\n\r\n", dl ? "YES !!!" : "No");
    xSemaphoreGive(s_guard);
}

const MutexInfo_t *MutexGuard_GetTable(int *count_out)
{
    if (count_out) *count_out = s_mutex_count;
    return s_mutexes;
}
