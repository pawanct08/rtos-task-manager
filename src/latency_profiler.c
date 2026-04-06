// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2026 pawanct08

/*=============================================================================
 * latency_profiler.c
 * Context switch latency measurement via DWT cycle counter.
 *=============================================================================*/

#include "latency_profiler.h"
#include "uart_cli.h"
#include <string.h>

/* Bucket upper bounds in µs (last bucket = everything above) */
static const uint32_t BUCKETS_US[LP_BUCKET_COUNT] = { 5, 10, 20, 50, 100, 200, UINT32_MAX };
static const char    *BUCKET_LABELS[LP_BUCKET_COUNT] = {
    "  0-5 us", " 5-10 us", "10-20 us", "20-50 us", "50-100us", "100-200", " 200+  "
};

static lp_stats_t s_stats;

/*---------------------------------------------------------------------------*/
void lp_init(void)
{
    lp_reset();
}

void lp_reset(void)
{
    memset(&s_stats, 0, sizeof(s_stats));
    s_stats.min_us = UINT32_MAX;
}

/*---------------------------------------------------------------------------*/
void lp_record(uint32_t cycles_elapsed)
{
    uint32_t us = cycles_elapsed / (LP_CPU_CLOCK_HZ / 1000000UL);

    /* Update min/max/sum */
    if (us < s_stats.min_us) s_stats.min_us = us;
    if (us > s_stats.max_us) s_stats.max_us = us;
    s_stats.sum_us += us;
    s_stats.count++;

    /* Bucket */
    for (int i = 0; i < LP_BUCKET_COUNT; i++) {
        if (us < BUCKETS_US[i]) {
            s_stats.buckets[i]++;
            break;
        }
    }

    /* Store raw sample (ring — overwrites oldest when full) */
    uint32_t idx = s_stats.sample_idx % LP_MAX_SAMPLES;
    s_stats.samples[idx] = (us > 0xFFFF) ? 0xFFFF : (uint16_t)us;
    s_stats.sample_idx++;
}

/*---------------------------------------------------------------------------*/
void lp_print_stats(void)
{
    if (s_stats.count == 0) {
        UART_PutStr("\r\n[Latency] No samples yet.\r\n");
        return;
    }

    uint32_t avg = s_stats.sum_us / s_stats.count;

    UART_PutStr("\r\n=== Context Switch Latency (DWT) ===\r\n");
    UART_Printf("  Samples : %lu\r\n",  (unsigned long)s_stats.count);
    UART_Printf("  Min     : %lu us\r\n", (unsigned long)s_stats.min_us);
    UART_Printf("  Max     : %lu us\r\n", (unsigned long)s_stats.max_us);
    UART_Printf("  Avg     : %lu us\r\n", (unsigned long)avg);
    UART_PutStr("\r\n  Histogram:\r\n");
    UART_PutStr("  +----------+-------+-------+\r\n");
    UART_PutStr("  | Bucket   | Count |   %   |\r\n");
    UART_PutStr("  +----------+-------+-------+\r\n");

    for (int i = 0; i < LP_BUCKET_COUNT; i++) {
        uint32_t pct = (s_stats.count > 0)
                       ? (s_stats.buckets[i] * 100) / s_stats.count
                       : 0;
        UART_Printf("  | %s | %5lu | %5lu |\r\n",
                    BUCKET_LABELS[i],
                    (unsigned long)s_stats.buckets[i],
                    (unsigned long)pct);
    }
    UART_PutStr("  +----------+-------+-------+\r\n\r\n");
}

/*---------------------------------------------------------------------------*/
void lp_dump_csv(void)
{
    /* Format: CSV header + one row per sample
     * Python reads this from UART log and plots with matplotlib */
    uint32_t total = (s_stats.sample_idx < LP_MAX_SAMPLES)
                     ? s_stats.sample_idx
                     : LP_MAX_SAMPLES;

    UART_PutStr("\r\nLATENCY_CSV_START\r\n");
    UART_PutStr("sample_us\r\n");
    for (uint32_t i = 0; i < total; i++) {
        uint32_t idx = (s_stats.sample_idx >= LP_MAX_SAMPLES)
                       ? (s_stats.sample_idx + i) % LP_MAX_SAMPLES
                       : i;
        UART_Printf("%u\r\n", (unsigned)s_stats.samples[idx]);
    }
    UART_PutStr("LATENCY_CSV_END\r\n\r\n");
}
