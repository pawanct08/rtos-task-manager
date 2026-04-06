// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 pawanct08

#include "autosar_os.h"
#include "uart_cli.h"
#include "task_manager.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

static AutosarAlarm_t s_alarms[MAX_ALARMS];
static int s_alarm_count = 0;

void AutosarOS_Init(void) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        memset(&s_alarms[i], 0, sizeof(AutosarAlarm_t));
        s_alarms[i].active = false;
    }
    s_alarm_count = 0;
}

static void TimerCallback(TimerHandle_t xTimer) {
    AlarmCallback_t cb = (AlarmCallback_t)pvTimerGetTimerID(xTimer);
    if (cb) cb();
}

StatusType AutosarOS_SetRelAlarm(const char *name, uint32_t period_ms,
                                  AlarmCallback_t cb) {
    if (s_alarm_count >= MAX_ALARMS) return E_OS_LIMIT;
    if (!name || !cb)               return E_OS_VALUE;

    AutosarAlarm_t *a = &s_alarms[s_alarm_count++];
    strncpy(a->name, name, sizeof(a->name) - 1);
    a->name[sizeof(a->name) - 1] = '\0';
    a->period_ms = period_ms;
    a->callback  = cb;
    a->active    = true;

    a->timer = xTimerCreate(name,
                            pdMS_TO_TICKS(period_ms),
                            pdTRUE,             /* auto-reload */
                            (void *)cb,
                            TimerCallback);
    if (a->timer == NULL) {
        s_alarm_count--;
        return E_OS_VALUE;
    }

    xTimerStart(a->timer, 0);
    return E_OK;
}

StatusType AutosarOS_CancelAlarm(const char *name) {
    for (int i = 0; i < s_alarm_count; i++) {
        if (s_alarms[i].active &&
            strncmp(s_alarms[i].name, name, sizeof(s_alarms[i].name)) == 0)
        {
            xTimerStop(s_alarms[i].timer, 0);
            s_alarms[i].active = false;
            return E_OK;
        }
    }
    return E_OS_ID;
}

bool AutosarOS_CheckSchedulability(const RMSEntry_t *tasks, uint32_t n) {
    if (n == 0) return true;

    double u_total = 0.0;
    for (uint32_t i = 0; i < n; i++) {
        u_total += ((double)tasks[i].wcet_us / (double)tasks[i].period_us);
    }

    /* RMS bound: n(2^(1/n) - 1) — precomputed for n=1..8, ln(2) beyond */
    static const double bounds[] = {
        1.000, 0.828, 0.779, 0.756, 0.743, 0.734, 0.728, 0.724
    };
    double limit = (n <= 8) ? bounds[n - 1] : 0.693;

    UART_Printf("[AUTOSAR] RMS schedulability: U=%.3f limit=%.3f → %s\r\n",
                u_total, limit,
                (u_total <= limit) ? "SCHEDULABLE" : "NOT SCHEDULABLE");

    return (u_total <= limit);
}

void AutosarOS_PrintMapping(void) {
    int task_count = 0;
    const TaskInfo_t *tbl = TaskManager_GetTable(&task_count);

    /* AUTOSAR category names */
    static const char *cat_str[] = {
        "Basic Task    ",
        "Extended Task ",
        "ISR Category 1",
        "ISR Category 2"
    };

    UART_PutStr("\r\n=== AUTOSAR OS Mapping ===\r\n");
    UART_PutStr("+----------------+----------------+--------+---------+---------+\r\n");
    UART_PutStr("| Task           | AUTOSAR Cat    | Period | Deadline| WCET    |\r\n");
    UART_PutStr("+----------------+----------------+--------+---------+---------+\r\n");

    for (int i = 0; i < task_count; i++) {
        const TaskInfo_t *t = &tbl[i];
        if (!t->active) continue;
        const char *cat = (t->autosar_cat <= 3) ? cat_str[t->autosar_cat]
                                                 : "Unknown       ";
        UART_Printf("| %-14s | %s | %5ums | %6ums | %5ums |\r\n",
                    t->name, cat,
                    (unsigned)t->period_ms,
                    (unsigned)t->deadline_ms,
                    (unsigned)t->wcet_ms);
    }
    UART_PutStr("+----------------+----------------+--------+---------+---------+\r\n");

    /* Alarm table */
    UART_PutStr("\r\n  Active Alarms:\r\n");
    UART_PutStr("  +----------------+----------+---------+\r\n");
    UART_PutStr("  | Name           | Period   | State   |\r\n");
    UART_PutStr("  +----------------+----------+---------+\r\n");
    bool any = false;
    for (int i = 0; i < s_alarm_count; i++) {
        if (s_alarms[i].active) {
            UART_Printf("  | %-14s | %6ums | Active  |\r\n",
                        s_alarms[i].name,
                        (unsigned)s_alarms[i].period_ms);
            any = true;
        }
    }
    if (!any) UART_PutStr("  | (none)         |          |         |\r\n");
    UART_PutStr("  +----------------+----------+---------+\r\n\r\n");

    /* RMS check for registered tasks */
    if (task_count > 0) {
        RMSEntry_t rms[8];
        int n = 0;
        for (int i = 0; i < task_count && n < 8; i++) {
            if (!tbl[i].active || tbl[i].period_ms == 0) continue;
            rms[n].name      = tbl[i].name;
            rms[n].period_us = tbl[i].period_ms * 1000UL;
            rms[n].wcet_us   = tbl[i].wcet_ms   * 1000UL;
            n++;
        }
        AutosarOS_CheckSchedulability(rms, (uint32_t)n);
    }
}

