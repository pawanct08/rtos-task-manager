#!/usr/bin/env python3
"""Generate comprehensive PDF documentation for the RTOS Task Manager project.

Usage:
    python scripts/gen_docs_pdf.py

Output:
    docs/rtos-task-manager-documentation.pdf
"""

import os
from fpdf import FPDF

DOCS_DIR = "docs"
OUTPUT_PDF = os.path.join(DOCS_DIR, "rtos-task-manager-documentation.pdf")

# ---------------------------------------------------------------------------
# PDF helper class
# ---------------------------------------------------------------------------

class DocPDF(FPDF):
    """Custom FPDF subclass providing header/footer and helpers."""

    def __init__(self):
        super().__init__(orientation="P", unit="mm", format="A4")
        self.set_auto_page_break(auto=True, margin=18)
        self.set_margins(20, 20, 20)
        self._section_title = ""

    def set_section(self, title):
        self._section_title = title

    def header(self):
        if self.page_no() == 1:
            return
        self.set_font("Helvetica", "I", 8)
        self.set_text_color(120, 120, 120)
        self.cell(0, 6, "RTOS Task Manager -- Project Documentation", align="L")
        self.cell(0, 6, self._section_title, align="R", new_x="LMARGIN", new_y="NEXT")
        self.set_draw_color(180, 180, 180)
        self.line(20, self.get_y(), 190, self.get_y())
        self.ln(2)
        self.set_text_color(0, 0, 0)

    def footer(self):
        if self.page_no() == 1:
            return
        self.set_y(-12)
        self.set_font("Helvetica", "I", 8)
        self.set_text_color(120, 120, 120)
        self.cell(0, 6, f"Page {self.page_no()}", align="C")
        self.set_text_color(0, 0, 0)

    # --- styled helpers ---

    def h1(self, text):
        self.set_section(text)
        self.set_font("Helvetica", "B", 18)
        self.set_text_color(30, 70, 130)
        self.ln(4)
        self.cell(0, 10, text, new_x="LMARGIN", new_y="NEXT")
        self.set_draw_color(30, 70, 130)
        self.line(20, self.get_y(), 190, self.get_y())
        self.ln(3)
        self.set_text_color(0, 0, 0)

    def h2(self, text):
        self.set_font("Helvetica", "B", 13)
        self.set_text_color(50, 100, 160)
        self.ln(3)
        self.cell(0, 8, text, new_x="LMARGIN", new_y="NEXT")
        self.ln(1)
        self.set_text_color(0, 0, 0)

    def h3(self, text):
        self.set_font("Helvetica", "B", 11)
        self.set_text_color(80, 80, 80)
        self.ln(2)
        self.cell(0, 7, text, new_x="LMARGIN", new_y="NEXT")
        self.set_text_color(0, 0, 0)

    def body(self, text):
        self.set_font("Helvetica", "", 10)
        self.multi_cell(0, 5.5, text)
        self.ln(1)

    def bullet(self, text, indent=5):
        self.set_font("Helvetica", "", 10)
        self.set_x(self.l_margin + indent)
        self.cell(5, 5.5, "-")
        self.multi_cell(0, 5.5, text)

    def code_block(self, lines):
        """Render a code block with grey background."""
        self.set_fill_color(240, 240, 240)
        self.set_font("Courier", "", 8)
        self.set_draw_color(200, 200, 200)
        for line in lines:
            self.cell(0, 4.8, line, fill=True, new_x="LMARGIN", new_y="NEXT",
                      border=0)
        self.set_fill_color(255, 255, 255)
        self.set_font("Helvetica", "", 10)
        self.ln(2)

    def kv_table(self, rows):
        """Key-value table with alternating rows."""
        self.set_font("Helvetica", "", 9)
        col_w = [55, 115]
        fill = False
        for k, v in rows:
            self.set_fill_color(230, 238, 255) if fill else self.set_fill_color(248, 248, 255)
            self.cell(col_w[0], 6, k, border=1, fill=True)
            self.cell(col_w[1], 6, v, border=1, fill=True, new_x="LMARGIN", new_y="NEXT")
            fill = not fill
        self.set_fill_color(255, 255, 255)
        self.ln(2)


# ---------------------------------------------------------------------------
# Section generators
# ---------------------------------------------------------------------------

def cover_page(pdf: DocPDF):
    pdf.add_page()
    # Large title block
    pdf.set_fill_color(30, 70, 130)
    pdf.rect(0, 0, 210, 80, "F")
    pdf.set_y(22)
    pdf.set_font("Helvetica", "B", 28)
    pdf.set_text_color(255, 255, 255)
    pdf.cell(0, 14, "RTOS Task Manager", align="C", new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "", 14)
    pdf.cell(0, 9, "Comprehensive Project Documentation", align="C",
             new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "I", 10)
    pdf.cell(0, 8, "FreeRTOS v10.5.1 -- ARM Cortex-M4 -- QEMU mps2-an386",
             align="C", new_x="LMARGIN", new_y="NEXT")
    pdf.set_text_color(0, 0, 0)
    pdf.set_y(100)
    pdf.set_font("Helvetica", "", 11)
    pdf.set_text_color(60, 60, 60)
    abstract = (
        "This document provides a full technical reference for the RTOS Task Manager "
        "project: a bare-metal FreeRTOS demonstration targeting ARM Cortex-M4 executed "
        "inside QEMU. It covers the software architecture, module-by-module API "
        "reference, build instructions, CLI command reference, BSP details, and "
        "contribution guidelines."
    )
    pdf.multi_cell(0, 6, abstract, align="J")
    pdf.ln(8)
    pdf.set_font("Helvetica", "B", 10)
    pdf.set_text_color(30, 70, 130)
    detail_rows = [
        ("Target", "ARM Cortex-M4F (mps2-an386 via QEMU)"),
        ("RTOS", "FreeRTOS V10.5.1, heap_4, CM4F port"),
        ("Toolchain", "arm-none-eabi-gcc + CMake 3.20+"),
        ("Build host", "Linux / macOS / WSL2"),
        ("Language", "C11"),
        ("License", "MIT"),
        ("Latest commit", "93cd9ef -- histogram + QEMU fixes"),
    ]
    pdf.set_text_color(0, 0, 0)
    pdf.kv_table(detail_rows)


def toc_page(pdf: DocPDF):
    pdf.add_page()
    pdf.set_section("Table of Contents")
    pdf.set_font("Helvetica", "B", 18)
    pdf.set_text_color(30, 70, 130)
    pdf.cell(0, 12, "Table of Contents", new_x="LMARGIN", new_y="NEXT")
    pdf.set_draw_color(30, 70, 130)
    pdf.line(20, pdf.get_y(), 190, pdf.get_y())
    pdf.ln(4)
    toc_entries = [
        ("1", "Project Overview", "3"),
        ("2", "Architecture", "5"),
        ("3", "Module: task_manager", "7"),
        ("4", "Module: mutex_guard", "10"),
        ("5", "Module: mem_pool", "12"),
        ("6", "Module: uart_cli", "14"),
        ("7", "Module: autosar_os (emulation)", "16"),
        ("8", "Module: latency_profiler", "18"),
        ("9", "Deadlock Stress Test", "20"),
        ("10", "Build System", "21"),
        ("11", "Board Support Package (BSP)", "23"),
        ("12", "Getting Started", "24"),
        ("13", "CLI Command Reference", "26"),
        ("14", "Contributing", "27"),
    ]
    pdf.set_font("Helvetica", "", 11)
    pdf.set_text_color(0, 0, 0)
    for num, title, page in toc_entries:
        pdf.set_font("Helvetica", "B", 10)
        pdf.cell(8, 7, num)
        pdf.set_font("Helvetica", "", 10)
        dots = "." * max(2, 62 - len(title))
        pdf.cell(0, 7, f"{title} {dots} {page}", new_x="LMARGIN", new_y="NEXT")


def section_overview(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("1. Project Overview")
    pdf.body(
        "The RTOS Task Manager is a fully self-contained FreeRTOS demonstration "
        "project for the ARM Cortex-M4. It is designed to run entirely inside the "
        "QEMU 'mps2-an386' machine, making it accessible on any development host "
        "without requiring physical hardware."
    )
    pdf.h2("Goals")
    goals = [
        "Demonstrate priority-based preemptive scheduling with FreeRTOS.",
        "Provide a reusable, memory-safe dynamic memory pool (mem_pool).",
        "Expose a UART-driven interactive CLI for runtime inspection.",
        "Emulate AUTOSAR OS concepts: alarms, schedular tables, runnable entities.",
        "Measure and report inter-task latency via a histogram-based profiler.",
        "Exercise mutex contention and deadlock detection under stress.",
    ]
    for g in goals:
        pdf.bullet(g)
    pdf.ln(2)
    pdf.h2("Repository Layout")
    pdf.kv_table([
        ("src/", "Application source files (C11)"),
        ("include/", "Public header files"),
        ("bsp/", "Board support: linker scripts, startup code"),
        ("cmake/", "CMake toolchain file for arm-none-eabi-gcc"),
        ("scripts/", "Helper shell/Python scripts (QEMU launch, PDF gen)"),
        ("docs/", "Generated documentation (PDF)"),
        ("latency_profiler.c/h", "Latency profiler implementation (root level)"),
    ])
    pdf.h2("Technology Stack")
    pdf.kv_table([
        ("FreeRTOS", "v10.5.1 -- heap_4 allocator, CM4F port"),
        ("Compiler", "arm-none-eabi-gcc (GCC 10+)"),
        ("Build system", "CMake 3.20+"),
        ("Emulator", "QEMU 7+ (mps2-an386 machine, ARM Cortex-M4)"),
        ("Debug", "GDB + QEMU GDB stub (port 1234)"),
        ("CI", "GitHub Actions (build + QEMU smoke test)"),
    ])
    pdf.h2("Key Design Decisions")
    pdf.h3("Static-first memory policy")
    pdf.body(
        "FreeRTOS heap_4 is used for task stacks allocated at startup. The custom "
        "mem_pool module provides O(1) pool allocation for fixed-size application "
        "objects, avoiding fragmentation during normal operation."
    )
    pdf.h3("Layered abstraction")
    pdf.body(
        "Hardware access is confined to the BSP layer (uart_cli -> UART MMIO, "
        "startup.c -> vector table). All higher-level modules depend only on "
        "FreeRTOS APIs and the BSP UART abstraction."
    )
    pdf.h3("Defensive coding")
    pdf.body(
        "Every public API validates its parameters and returns a typed status code "
        "(TM_OK / TM_ERR_*). NULL pointer guards and range checks are present "
        "throughout, consistent with MISRA C:2012 advisory guidance."
    )


def section_architecture(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("2. Architecture")
    pdf.body(
        "The system is structured into five layers. Each layer may only call "
        "downward; no upward dependencies are permitted."
    )
    pdf.h2("Layer Diagram")
    pdf.code_block([
        "+--------------------------------------------------+",
        "|  Application Layer                               |",
        "|  main.c  autosar_os.c  deadlock_stress tasks     |",
        "+--------------------------------------------------+",
        "|  Service Layer                                   |",
        "|  task_manager  mutex_guard  latency_profiler      |",
        "+--------------------------------------------------+",
        "|  Infrastructure Layer                            |",
        "|  mem_pool  uart_cli                              |",
        "+--------------------------------------------------+",
        "|  RTOS Layer                                      |",
        "|  FreeRTOS v10.5.1 (heap_4, CM4F port)           |",
        "+--------------------------------------------------+",
        "|  BSP / Hardware Abstraction Layer                |",
        "|  startup.c  linker scripts  UART MMIO            |",
        "+--------------------------------------------------+",
        "|  Hardware / Emulator                             |",
        "|  QEMU mps2-an386  (ARM Cortex-M4, 256 kB SRAM)  |",
        "+--------------------------------------------------+",
    ])
    pdf.h2("Task Inventory")
    pdf.kv_table([
        ("vTaskManager", "Monitors all registered tasks; 500 ms periodic"),
        ("vMutexStressTask (x2)", "Contend on a shared mutex; detect deadlock"),
        ("vAutosar_OsTask", "Fires AUTOSAR alarms and schedular table steps"),
        ("vLatencyTask", "Measures scheduler wake-up latency, updates histogram"),
        ("vCLI_Task", "Reads UART input, dispatches CLI commands"),
        ("vStatPrinter", "Prints FreeRTOS run-time stats every 5 s"),
    ])
    pdf.h2("Memory Map (QEMU mps2-an386)")
    pdf.kv_table([
        ("Flash (ROM)", "0x00000000 -- 0x003FFFFF (4 MB)"),
        ("SRAM", "0x20000000 -- 0x2003FFFF (256 kB)"),
        ("UART0 base", "0x40004000"),
        ("FreeRTOS heap", "Starts after .bss; size = configTOTAL_HEAP_SIZE"),
        ("Stack (main)", "Top of SRAM (grows downward)"),
    ])
    pdf.h2("Interrupt Priority Scheme")
    pdf.body(
        "The Cortex-M4 NVIC is configured with 4 bits of priority (16 levels). "
        "FreeRTOS uses priorities 0xE0-0xFF for its tick and yield interrupts. "
        "The UART Rx interrupt is assigned priority 0xC0 so it can safely call "
        "FreeRTOS ISR-safe APIs (xQueueSendFromISR)."
    )
    pdf.h2("Scheduler Configuration")
    pdf.kv_table([
        ("configUSE_PREEMPTION", "1 (preemptive)"),
        ("configUSE_TIME_SLICING", "1 (round-robin at same priority)"),
        ("configTICK_RATE_HZ", "1000 (1 ms tick)"),
        ("configMAX_PRIORITIES", "8"),
        ("configMINIMAL_STACK_SIZE", "128 words (512 bytes)"),
        ("configTOTAL_HEAP_SIZE", "32768 bytes (32 kB)"),
        ("configUSE_MUTEXES", "1"),
        ("configUSE_RECURSIVE_MUTEXES", "1"),
        ("configUSE_COUNTING_SEMAPHORES", "1"),
        ("configGENERATE_RUN_TIME_STATS", "1"),
        ("configUSE_TRACE_FACILITY", "1"),
    ])


def section_task_manager(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("3. Module: task_manager")
    pdf.body(
        "task_manager provides a registry of FreeRTOS tasks with structured "
        "metadata, health monitoring, and a watchdog mechanism. It is the "
        "central bookkeeping layer for all application tasks."
    )
    pdf.h2("Header: include/task_manager.h")
    pdf.h3("Status codes")
    pdf.code_block([
        "typedef enum {",
        "    TM_OK            = 0,",
        "    TM_ERR_FULL      = -1,  /* registry is full */",
        "    TM_ERR_NOT_FOUND = -2,  /* handle not in registry */",
        "    TM_ERR_NULL      = -3,  /* NULL pointer argument */",
        "    TM_ERR_INVALID   = -4,  /* invalid parameter value */",
        "} TM_Status_t;",
    ])
    pdf.h3("Task descriptor")
    pdf.code_block([
        "typedef struct {",
        "    TaskHandle_t  handle;          /* FreeRTOS handle */",
        "    const char   *name;            /* human-readable name */",
        "    UBaseType_t   priority;        /* current priority */",
        "    uint32_t      stack_hwm;       /* stack high-water mark (words) */",
        "    uint32_t      wakeup_count;    /* incremented each wake-up */",
        "    TickType_t    last_wake_tick;  /* tick of last wake-up */",
        "    uint8_t       watchdog_fed;    /* set by task, cleared by monitor */",
        "} TM_TaskInfo_t;",
    ])
    pdf.h2("Public API")
    api_rows = [
        ("TM_Init()", "Initialise the registry; must be called once before any task registers."),
        ("TM_Register(handle, name, prio)", "Add a task to the registry. Returns TM_ERR_FULL if capacity exceeded."),
        ("TM_Unregister(handle)", "Remove a task (called from vTaskDelete hook)."),
        ("TM_FeedWatchdog(handle)", "Task calls this each cycle to signal liveness."),
        ("TM_GetInfo(handle, info_out)", "Copy task metadata into caller-supplied TM_TaskInfo_t."),
        ("TM_PrintAll()", "Dump all registered tasks to UART via uart_cli."),
        ("TM_MonitorTask(arg)", "FreeRTOS task function: wakes every 500 ms, checks watchdogs."),
    ]
    pdf.set_font("Helvetica", "B", 9)
    pdf.cell(55, 6, "Function", border=1)
    pdf.cell(115, 6, "Description", border=1, new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "", 9)
    fill = False
    for fn, desc in api_rows:
        pdf.set_fill_color(240, 245, 255) if fill else pdf.set_fill_color(252, 252, 252)
        pdf.cell(55, 5.5, fn, border=1, fill=True)
        pdf.cell(115, 5.5, desc, border=1, fill=True, new_x="LMARGIN", new_y="NEXT")
        fill = not fill
    pdf.set_fill_color(255, 255, 255)
    pdf.ln(3)
    pdf.h2("Implementation Notes")
    pdf.body(
        "The registry is a fixed-size array of TM_TaskInfo_t (size configurable "
        "via TM_MAX_TASKS, default 10). Access is protected by a FreeRTOS mutex "
        "so TM_Register / TM_Unregister are safe to call from multiple tasks "
        "simultaneously."
    )
    pdf.h3("Watchdog logic")
    pdf.body(
        "TM_MonitorTask wakes every 500 ms via vTaskDelayUntil. It iterates all "
        "registered tasks and checks whether watchdog_fed is non-zero. Any task "
        "that has not fed the watchdog is logged as 'STALLED' via uart_cli. The "
        "flag is then cleared for the next interval."
    )
    pdf.h3("Stack high-water mark")
    pdf.body(
        "TM_MonitorTask calls uxTaskGetStackHighWaterMark() on each handle and "
        "updates stack_hwm. If the high-water mark drops below a configurable "
        "threshold (default 64 words / 256 bytes), a stack overflow warning is "
        "emitted."
    )
    pdf.h2("Example Usage")
    pdf.code_block([
        "/* In task creation code: */",
        "TaskHandle_t hMyTask;",
        "xTaskCreate(vMyTask, \"MyTask\", 256, NULL, 3, &hMyTask);",
        "TM_Register(hMyTask, \"MyTask\", 3);",
        "",
        "/* Inside vMyTask main loop: */",
        "for (;;) {",
        "    TM_FeedWatchdog(hMyTask);",
        "    /* ... do work ... */",
        "    vTaskDelay(pdMS_TO_TICKS(100));",
        "}",
    ])


def section_mutex_guard(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("4. Module: mutex_guard")
    pdf.body(
        "mutex_guard implements a scoped mutex wrapper inspired by C++ RAII. "
        "It pairs an acquire call with an automatic release on scope exit, "
        "reducing the risk of forgetting to release a mutex on early return or "
        "error paths."
    )
    pdf.h2("Header: include/mutex_guard.h")
    pdf.code_block([
        "typedef struct {",
        "    SemaphoreHandle_t mutex;   /* the guarded mutex */",
        "    BaseType_t        locked;  /* pdTRUE if currently held */",
        "} MutexGuard_t;",
        "",
        "/* Initialise guard (does NOT take the mutex). */",
        "void MG_Init(MutexGuard_t *guard, SemaphoreHandle_t mutex);",
        "",
        "/* Attempt to acquire.  Returns pdTRUE on success. */",
        "BaseType_t MG_Lock(MutexGuard_t *guard, TickType_t timeout_ticks);",
        "",
        "/* Release if held (idempotent). */",
        "void MG_Unlock(MutexGuard_t *guard);",
        "",
        "/* Lock + execute callback + unlock atomically. */",
        "BaseType_t MG_WithLock(MutexGuard_t *guard,",
        "                        void (*fn)(void *ctx), void *ctx,",
        "                        TickType_t timeout);",
    ])
    pdf.h2("Deadlock Detection")
    pdf.body(
        "The stress test in main.c creates two tasks that each hold one mutex "
        "while trying to acquire the other. mutex_guard detects the timeout "
        "(MG_Lock returning pdFALSE after portMAX_DELAY) and reports the "
        "deadlock candidate via uart_cli. This is an application-level detection "
        "strategy, not a kernel-level one."
    )
    pdf.h2("Example Usage")
    pdf.code_block([
        "MutexGuard_t guard;",
        "SemaphoreHandle_t hMutex = xSemaphoreCreateMutex();",
        "MG_Init(&guard, hMutex);",
        "",
        "/* Simple lock / unlock: */",
        "if (MG_Lock(&guard, pdMS_TO_TICKS(50)) == pdTRUE) {",
        "    /* protected section */",
        "    shared_counter++;",
        "    MG_Unlock(&guard);",
        "}",
        "",
        "/* Callback style: */",
        "MG_WithLock(&guard, my_callback, &my_data, pdMS_TO_TICKS(100));",
    ])
    pdf.h2("Thread Safety Notes")
    pdf.body(
        "A MutexGuard_t instance must NOT be shared between tasks. Each task "
        "should maintain its own MutexGuard_t that wraps the same "
        "SemaphoreHandle_t. Sharing a guard would allow one task to release "
        "a mutex held by another, which violates FreeRTOS mutex ownership rules."
    )


def section_mem_pool(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("5. Module: mem_pool")
    pdf.body(
        "mem_pool implements a fixed-size block memory allocator backed by a "
        "statically allocated array. Allocation and deallocation are both O(1) "
        "using a free-list. No heap fragmentation is possible because all blocks "
        "are the same size."
    )
    pdf.h2("Header: include/mem_pool.h")
    pdf.code_block([
        "/* Opaque pool handle (defined in mem_pool.c). */",
        "typedef struct MemPool MemPool_t;",
        "",
        "/*",
        " * Create a pool from a caller-supplied buffer.",
        " *   buf        -- pointer to raw byte buffer",
        " *   buf_size   -- total size of buf in bytes",
        " *   block_size -- size of each allocation block",
        " * Returns a pointer to the pool metadata (stored inside buf),",
        " * or NULL on invalid arguments.",
        " */",
        "MemPool_t *MP_Create(void *buf, size_t buf_size, size_t block_size);",
        "",
        "/* Allocate one block.  Returns NULL if pool is exhausted. */",
        "void *MP_Alloc(MemPool_t *pool);",
        "",
        "/* Return block to pool.  ptr must have been obtained from MP_Alloc. */",
        "void  MP_Free(MemPool_t *pool, void *ptr);",
        "",
        "/* Current number of free blocks remaining. */",
        "size_t MP_FreeCount(const MemPool_t *pool);",
    ])
    pdf.h2("Free-list Design")
    pdf.body(
        "The pool stores a free-list as a NULL-terminated singly-linked list of "
        "pointers overlaid on the unused portions of free blocks. MP_Alloc pops "
        "the head of the list; MP_Free pushes the returned block back onto the "
        "head. Both operations are a single pointer read/write, achieving true "
        "O(1) performance."
    )
    pdf.h2("Thread Safety")
    pdf.body(
        "MP_Alloc and MP_Free are NOT intrinsically thread-safe -- the caller "
        "must protect pool operations with a mutex if multiple tasks share the "
        "same pool. A convenience macro MP_ALLOC_SAFE(pool, mutex) and "
        "MP_FREE_SAFE(pool, ptr, mutex) are provided to simplify this pattern."
    )
    pdf.h2("Typical Usage")
    pdf.code_block([
        "#define POOL_BLOCKS  16",
        "#define BLOCK_SIZE   64",
        "static uint8_t pool_buf[POOL_BLOCKS * (BLOCK_SIZE + sizeof(void*))];",
        "",
        "MemPool_t *my_pool = MP_Create(pool_buf, sizeof(pool_buf), BLOCK_SIZE);",
        "configASSERT(my_pool != NULL);",
        "",
        "void *p = MP_Alloc(my_pool);",
        "if (p) {",
        "    memset(p, 0, BLOCK_SIZE);",
        "    /* use p */",
        "    MP_Free(my_pool, p);",
        "}",
    ])
    pdf.h2("Capacity Calculation")
    pdf.body(
        "The number of usable blocks in a buffer of size B with block size S is:"
    )
    pdf.code_block([
        "usable_blocks = floor( (B - sizeof(MemPool_t)) / max(S, sizeof(void*)) )",
    ])
    pdf.body(
        "The MP_Create function performs this calculation internally and stores "
        "the result in the pool metadata header at the start of the buffer."
    )


def section_uart_cli(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("6. Module: uart_cli")
    pdf.body(
        "uart_cli provides a lightweight interactive command-line interface over "
        "the mps2-an386 UART0. It supports command registration, tokenisation, "
        "and dispatch. Output is formatted for a standard 80-column terminal."
    )
    pdf.h2("Header: include/uart_cli.h")
    pdf.code_block([
        "/* Callback type for CLI commands.",
        "   argc/argv follow the POSIX convention. */",
        "typedef void (*CLI_CommandFn)(int argc, char **argv);",
        "",
        "/* Initialise the UART (baud = 115200, 8N1). */",
        "void CLI_Init(void);",
        "",
        "/* Register a command.  name must remain valid indefinitely. */",
        "void CLI_Register(const char *name, CLI_CommandFn fn,",
        "                  const char *help);",
        "",
        "/* Print a formatted string to UART (printf-style). */",
        "void CLI_Printf(const char *fmt, ...) __attribute__((format(printf,1,2)));",
        "",
        "/* Print a raw string (no formatting). */",
        "void CLI_Puts(const char *s);",
        "",
        "/* FreeRTOS task function -- reads UART, dispatches commands. */",
        "void vCLI_Task(void *arg);",
    ])
    pdf.h2("UART Hardware Configuration")
    pdf.kv_table([
        ("Base address", "0x40004000 (CMSDK APB UART)"),
        ("Baud rate", "115200"),
        ("Data bits", "8"),
        ("Stop bits", "1"),
        ("Parity", "None"),
        ("Flow control", "None"),
        ("Clock", "25 MHz (QEMU mps2-an386 default)"),
    ])
    pdf.h2("Command Dispatch Flow")
    pdf.code_block([
        "1. vCLI_Task blocks on UART Rx queue (interrupt-driven).",
        "2. Characters echo'd back; backspace supported.",
        "3. On '\\r' or '\\n': line is tokenised by whitespace.",
        "4. First token is looked up in the command table.",
        "5. Matching handler called with (argc, argv).",
        "6. If no match: 'Unknown command. Type \"help\".' printed.",
    ])
    pdf.h2("Built-in Commands")
    pdf.kv_table([
        ("help", "List all registered commands with their help strings"),
        ("tasks", "Show all registered tasks (via TM_PrintAll)"),
        ("stats", "Print FreeRTOS run-time statistics"),
        ("pool", "Show mem_pool free-block count"),
        ("lat", "Print latency histogram summary"),
        ("reset", "Trigger a software reset via SCB->AIRCR"),
    ])
    pdf.h2("Adding a Custom Command")
    pdf.code_block([
        "static void cmd_blink(int argc, char **argv) {",
        "    int count = (argc > 1) ? atoi(argv[1]) : 3;",
        "    CLI_Printf(\"Blinking %d times\\n\", count);",
        "    /* ... hardware action ... */",
        "}",
        "",
        "/* In main() or task init: */",
        "CLI_Register(\"blink\", cmd_blink, \"blink [n] -- blink LED n times\");",
    ])


def section_autosar_os(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("7. Module: autosar_os (emulation)")
    pdf.body(
        "autosar_os emulates a subset of AUTOSAR OS concepts on top of FreeRTOS. "
        "It is an educational layer -- not a certified AUTOSAR stack -- that "
        "demonstrates alarms, schedule tables, and runnable entities using "
        "standard FreeRTOS primitives."
    )
    pdf.h2("Concepts Emulated")
    pdf.kv_table([
        ("Alarm", "One-shot or cyclic timer that fires a callback after N ticks"),
        ("Schedule Table", "Ordered list of expiry points each with a callback"),
        ("Runnable Entity", "Function called at a specific schedule-table offset"),
        ("Counter (OsCounter)", "Wraps xTaskGetTickCount() as the AUTOSAR counter"),
        ("Task activation", "Conceptually maps to xTaskNotifyGive / task resume"),
    ])
    pdf.h2("Header: include/autosar_os.h")
    pdf.code_block([
        "/* Alarm handle */",
        "typedef uint8_t AlarmType;",
        "",
        "/* Return codes */",
        "typedef enum { E_OK=0, E_OS_ID, E_OS_VALUE, E_OS_STATE } StatusType;",
        "",
        "/* Set a relative alarm (fires after 'increment' counter ticks). */",
        "StatusType SetRelAlarm(AlarmType id, TickType_t increment,",
        "                       TickType_t cycle);",
        "",
        "/* Cancel an active alarm. */",
        "StatusType CancelAlarm(AlarmType id);",
        "",
        "/* Start a schedule table relative to now. */",
        "StatusType StartScheduleTableRel(uint8_t table_id,",
        "                                  TickType_t offset);",
        "",
        "/* Stop a running schedule table. */",
        "StatusType StopScheduleTable(uint8_t table_id);",
        "",
        "/* Internal task that drives alarm/schedule-table processing. */",
        "void vAutosar_OsTask(void *arg);",
    ])
    pdf.h2("Alarm Implementation")
    pdf.body(
        "Each alarm is stored in a small descriptor (expiry tick, cycle, callback). "
        "vAutosar_OsTask wakes every system tick, compares all active alarm expiry "
        "times against xTaskGetTickCount(), and fires callbacks whose time has "
        "come. Cyclic alarms are automatically re-armed."
    )
    pdf.h2("Schedule Table Implementation")
    pdf.body(
        "A schedule table is a sorted array of (offset, callback) pairs. "
        "vAutosar_OsTask maintains a 'next expiry' pointer into the array. When "
        "the current tick reaches the next expiry, the callback fires and the "
        "pointer advances. When the table wraps, it repeats from offset 0."
    )
    pdf.h2("Limitations vs. Real AUTOSAR OS")
    notes = [
        "No stack isolation between runnables (all share the vAutosar_OsTask stack).",
        "No resource ceiling protocol -- resource management is the application's responsibility.",
        "Counter resolution == FreeRTOS tick (1 ms) -- cannot emulate sub-ms counters.",
        "No OS hooks (PreTaskHook, PostTaskHook) -- only alarm expiry callbacks.",
        "No error handling tasks -- StatusType is returned but not propagated to an ErrorHook.",
    ]
    for n in notes:
        pdf.bullet(n)


def section_latency_profiler(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("8. Module: latency_profiler")
    pdf.body(
        "The latency profiler measures the schedule latency of a high-priority "
        "FreeRTOS task: the time between when the task theoretically should have "
        "run (requested wake-up time) and when it actually began executing. "
        "Results are bucketed into a histogram for statistical analysis."
    )
    pdf.h2("Files")
    pdf.kv_table([
        ("latency_profiler.c", "Implementation (root of repository)"),
        ("latency_profiler.h", "Public API (root of repository)"),
    ])
    pdf.h2("Header: latency_profiler.h")
    pdf.code_block([
        "#define LP_BUCKET_COUNT   16",
        "#define LP_BUCKET_WIDTH_US 10   /* us per bucket */",
        "",
        "typedef struct {",
        "    uint32_t buckets[LP_BUCKET_COUNT];  /* histogram counts */",
        "    uint32_t overflow;   /* samples >= max bucket */",
        "    uint32_t total;      /* total samples collected */",
        "    uint32_t sum_us;     /* sum of all latency values */",
        "} LP_Histogram_t;",
        "",
        "void LP_Init(LP_Histogram_t *h);",
        "void LP_RecordSample(LP_Histogram_t *h, uint32_t latency_us);",
        "void LP_PrintHistogram(const LP_Histogram_t *h);",
        "uint32_t LP_MeanUs(const LP_Histogram_t *h);",
        "uint32_t LP_MaxBucketUs(const LP_Histogram_t *h);",
    ])
    pdf.h2("Histogram Bucket Layout")
    pdf.body(
        "With LP_BUCKET_COUNT=16 and LP_BUCKET_WIDTH_US=10, latencies are "
        "bucketed as follows:"
    )
    bucket_rows = [(f"Bucket {i}", f"{i*10} us -- {(i+1)*10-1} us") for i in range(8)]
    bucket_rows += [("...", "..."), ("Bucket 15", "150 us -- 159 us"), ("overflow", ">= 160 us")]
    pdf.kv_table(bucket_rows)
    pdf.h2("LP_PrintHistogram output format")
    pdf.body("Result of commit 93cd9ef: bar-graph replaced by numeric histogram table.")
    pdf.code_block([
        "=== Latency Histogram (us) ===",
        "Bucket  Range(us)   Count",
        "     0   0 -   9:     245",
        "     1  10 -  19:      87",
        "     2  20 -  29:      12",
        "   ...  ...         ...",
        "    15  150- 159:       0",
        "Overflow (>=160us):    0",
        "Total samples:       344",
        "Mean latency:          5 us",
    ])
    pdf.h2("Measurement Method")
    pdf.body(
        "vLatencyTask calls xTaskGetTickCount() to record the intended wake-up "
        "tick, then calls vTaskDelayUntil(). After waking, it reads "
        "xTaskGetTickCount() again and computes the difference (converted to "
        "microseconds using configTICK_RATE_HZ). The result is passed to "
        "LP_RecordSample."
    )
    pdf.body(
        "Note: The QEMU environment does not emulate real Cortex-M4 timing. "
        "Measured latencies reflect QEMU's host-scheduling behaviour rather than "
        "silicon timing. Use a real Cortex-M4 board for production timing data."
    )


def section_deadlock_stress(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("9. Deadlock Stress Test")
    pdf.body(
        "The deadlock stress test exercises mutex_guard's timeout detection by "
        "creating a classic deadlock scenario: two tasks, two mutexes, each task "
        "holding one mutex while attempting to acquire the other."
    )
    pdf.h2("Task Structure")
    pdf.code_block([
        "/* Task A: holds mutex_1, tries to get mutex_2 */",
        "void vDeadlockTask_A(void *arg) {",
        "    MG_Lock(&guard_1, portMAX_DELAY);   /* acquires mutex_1 */",
        "    vTaskDelay(pdMS_TO_TICKS(10));       /* deliberate delay */",
        "    if (MG_Lock(&guard_2, pdMS_TO_TICKS(200)) != pdTRUE) {",
        "        CLI_Printf(\"[DEADLOCK] Task A: timeout waiting for mutex_2\\n\");",
        "    }",
        "    MG_Unlock(&guard_1);",
        "    MG_Unlock(&guard_2);",
        "}",
        "",
        "/* Task B: holds mutex_2, tries to get mutex_1 */",
        "void vDeadlockTask_B(void *arg) {",
        "    MG_Lock(&guard_2, portMAX_DELAY);   /* acquires mutex_2 */",
        "    vTaskDelay(pdMS_TO_TICKS(10));",
        "    if (MG_Lock(&guard_1, pdMS_TO_TICKS(200)) != pdTRUE) {",
        "        CLI_Printf(\"[DEADLOCK] Task B: timeout waiting for mutex_1\\n\");",
        "    }",
        "    MG_Unlock(&guard_2);",
        "    MG_Unlock(&guard_1);",
        "}",
    ])
    pdf.h2("Expected Output")
    pdf.code_block([
        "[DEADLOCK] Task A: timeout waiting for mutex_2",
        "[DEADLOCK] Task B: timeout waiting for mutex_1",
        "[TM_MONITOR] Task vDeadlockTask_A: STALLED (watchdog not fed)",
        "[TM_MONITOR] Task vDeadlockTask_B: STALLED (watchdog not fed)",
    ])
    pdf.h2("Resolution Strategy")
    pdf.body(
        "In a production system, the recommended resolution strategies are: "
        "(1) define a global lock ordering and always acquire in that order, "
        "(2) use try-lock (timeout) instead of blocking indefinitely, and "
        "(3) use a resource ceiling protocol (AUTOSAR OS resource management). "
        "The stress test demonstrates strategy (2): timed locks allow the system "
        "to detect and report the deadlock rather than hanging permanently."
    )


def section_build_system(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("10. Build System")
    pdf.body(
        "The project uses CMake with a cross-compilation toolchain file targeting "
        "arm-none-eabi-gcc. A single CMakeLists.txt at the repo root drives the "
        "entire build."
    )
    pdf.h2("Prerequisites")
    pdf.kv_table([
        ("CMake", ">= 3.20"),
        ("arm-none-eabi-gcc", ">= 10.x (GCC 12 recommended)"),
        ("arm-none-eabi-objcopy", "Included with GCC toolchain"),
        ("QEMU", ">= 7.0 (qemu-system-arm)"),
        ("Python 3", ">= 3.8 (for scripts only -- not needed to build)"),
        ("make / ninja", "Any CMake-compatible generator"),
    ])
    pdf.h2("Build Commands")
    pdf.code_block([
        "# Configure (from repo root):",
        "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake",
        "",
        "# Build:",
        "cmake --build build -j$(nproc)",
        "",
        "# Output:",
        "#   build/rtos_task_manager.elf   -- ELF with debug symbols",
        "#   build/rtos_task_manager.bin   -- raw binary for flashing",
        "",
        "# Optional: size report",
        "arm-none-eabi-size build/rtos_task_manager.elf",
    ])
    pdf.h2("CMake Variables")
    pdf.kv_table([
        ("CMAKE_TOOLCHAIN_FILE", "cmake/arm-none-eabi.cmake -- must be set"),
        ("CMAKE_BUILD_TYPE", "Debug (default) | Release | MinSizeRel"),
        ("FREERTOS_PORT", "GCC/ARM_CM4F (default, do not change for QEMU)"),
    ])
    pdf.h2("cmake/arm-none-eabi.cmake")
    pdf.body(
        "The toolchain file sets CMAKE_SYSTEM_NAME=Generic, locates "
        "arm-none-eabi-gcc / arm-none-eabi-g++ / arm-none-eabi-objcopy, and "
        "configures the following compiler flags:"
    )
    pdf.code_block([
        "-mcpu=cortex-m4    # target CPU",
        "-mthumb            # Thumb-2 instruction set",
        "-mfpu=fpv4-sp-d16  # Cortex-M4F single-precision FPU",
        "-mfloat-abi=hard   # hardware FP calling convention",
        "-ffunction-sections -fdata-sections  # enable dead-code elimination",
        "-Wall -Wextra -Werror  # treat warnings as errors",
    ])
    pdf.h2("Linker Script")
    pdf.body(
        "bsp/lm3s6965evb.ld (for QEMU lm3s6965evb) and mps2_an386.ld (for "
        "mps2-an386) define the memory regions. The default CMake build uses "
        "mps2_an386.ld. FLASH starts at 0x00000000 (4 MB), SRAM at 0x20000000 "
        "(256 kB). The .ccm section is unused (no CCM RAM on mps2-an386)."
    )
    pdf.h2("Running Under QEMU")
    pdf.code_block([
        "# scripts/run_qemu.sh (auto-generated path to ELF):",
        "qemu-system-arm \\",
        "    -M mps2-an386 \\",
        "    -cpu cortex-m4 \\",
        "    -kernel build/rtos_task_manager.elf \\",
        "    -nographic \\",
        "    -serial mon:stdio",
        "",
        "# Or directly:",
        "bash scripts/run_qemu.sh",
    ])


def section_bsp(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("11. Board Support Package (BSP)")
    pdf.body(
        "The BSP layer initialises the Cortex-M4 processor, sets up the vector "
        "table, and provides the minimal peripheral drivers needed by the "
        "application (UART0 on mps2-an386)."
    )
    pdf.h2("bsp/startup.c")
    pdf.body(
        "startup.c implements the Reset_Handler and provides weak definitions of "
        "all Cortex-M exception and NVIC interrupt handlers. The Reset_Handler:"
    )
    pdf.code_block([
        "1. Copies .data section from Flash to SRAM.",
        "2. Zero-fills .bss section.",
        "3. Calls SystemInit() (optional clock / MPU setup).",
        "4. Calls main().",
        "5. Infinite loop on unexpected main() return.",
    ])
    pdf.h2("Vector Table")
    pdf.body(
        "The vector table is defined as a const array in the .isr_vector section "
        "and placed at the start of Flash by the linker script. The table "
        "includes all 16 Cortex-M4 core exceptions plus 32 NVIC external "
        "interrupt vectors usable on mps2-an386."
    )
    pdf.h2("UART0 Driver (inside uart_cli.c)")
    pdf.body(
        "uart_cli.c directly accesses the CMSDK APB UART registers at 0x40004000. "
        "The baud rate divisor is computed as: divisor = UART_CLK / baud_rate. "
        "With UART_CLK = 25 MHz and baud_rate = 115200: divisor = 217 (0xD9)."
    )
    pdf.kv_table([
        ("UART DATA (0x000)", "Transmit/receive data register (bits 7:0)"),
        ("UART STATE (0x004)", "Bit 0: TX full; bit 1: RX full; bit 3: TX overrun"),
        ("UART CTRL (0x008)", "Bit 0: TX enable; bit 1: RX enable; bit 2: TX IRQ en"),
        ("UART BAUDDIV (0x010)", "Baud rate divisor"),
    ])


def section_getting_started(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("12. Getting Started")
    pdf.body(
        "This section walks through installing dependencies, cloning the "
        "repository, building, and running the firmware in QEMU."
    )
    pdf.h2("Step 1: Install Dependencies (Ubuntu / Debian / WSL2)")
    pdf.code_block([
        "sudo apt update",
        "sudo apt install -y \\",
        "    cmake ninja-build \\",
        "    gcc-arm-none-eabi binutils-arm-none-eabi \\",
        "    qemu-system-arm",
        "",
        "# Verify:",
        "arm-none-eabi-gcc --version",
        "qemu-system-arm --version",
    ])
    pdf.h2("Step 2: Clone and Initialise")
    pdf.code_block([
        "git clone https://github.com/pawanct08/rtos-task-manager.git",
        "cd rtos-task-manager",
        "",
        "# Run optional setup script (installs Python deps etc.):",
        "bash scripts/setup.sh",
    ])
    pdf.h2("Step 3: Build")
    pdf.code_block([
        "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake \\",
        "      -DCMAKE_BUILD_TYPE=Debug",
        "cmake --build build -j$(nproc)",
        "",
        "# Expect:",
        "# [100%] Linking C executable rtos_task_manager.elf",
        "# arm-none-eabi-size output...",
    ])
    pdf.h2("Step 4: Run in QEMU")
    pdf.code_block([
        "bash scripts/run_qemu.sh",
        "",
        "# Or manually:",
        "qemu-system-arm -M mps2-an386 -cpu cortex-m4 \\",
        "    -kernel build/rtos_task_manager.elf \\",
        "    -nographic -serial mon:stdio",
    ])
    pdf.body(
        "You should see boot messages followed by the RTOS scheduler starting. "
        "Type 'help' at the prompt and press Enter to list available CLI commands."
    )
    pdf.h2("Step 5: Debug with GDB")
    pdf.code_block([
        "# Terminal 1 -- start QEMU with GDB stub:",
        "bash scripts/debug_qemu.sh   # or:",
        "qemu-system-arm -M mps2-an386 -cpu cortex-m4 \\",
        "    -kernel build/rtos_task_manager.elf \\",
        "    -nographic -serial mon:stdio -S -gdb tcp::1234",
        "",
        "# Terminal 2 -- connect GDB:",
        "arm-none-eabi-gdb build/rtos_task_manager.elf",
        "(gdb) target remote :1234",
        "(gdb) break main",
        "(gdb) continue",
    ])
    pdf.h2("Expected Boot Output")
    pdf.code_block([
        "RTOS Task Manager v1.0 -- FreeRTOS v10.5.1",
        "Heap: 32768 bytes   Free: 29184 bytes",
        "[TM] Registered: vLatencyTask",
        "[TM] Registered: vCLI_Task",
        "[TM] Registered: vAutosar_OsTask",
        "[TM] Registered: vDeadlockTask_A",
        "[TM] Registered: vDeadlockTask_B",
        "[AUTOSAR] Alarm 0 armed  (cycle=500 ticks)",
        "Scheduler started.",
        "> ",
    ])


def section_cli_reference(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("13. CLI Command Reference")
    pdf.body(
        "All commands are entered at the '> ' prompt over UART at 115200 baud. "
        "Arguments are whitespace-separated. Commands are case-sensitive."
    )
    commands = [
        ("help", "", "List all registered commands and their descriptions."),
        ("tasks", "", "Print the task registry: name, priority, stack HWM, watchdog state."),
        ("stats", "", "Print FreeRTOS run-time statistics (task CPU usage % since boot)."),
        ("pool", "", "Show mem_pool diagnostics: capacity, free blocks, allocation count."),
        ("lat", "", "Print the latency histogram table and mean latency."),
        ("lat reset", "reset", "Clear the histogram and restart collection."),
        ("reset", "", "Trigger a software reset via SCB->AIRCR.SYSRESETREQ."),
        ("alarm", "<id> <ticks> <cycle>", "Arm AUTOSAR alarm <id> to fire after <ticks>, repeating every <cycle>."),
        ("sched", "<table> <offset>", "Start schedule table <table> at <offset> ticks from now."),
        ("sched stop", "<table>", "Stop schedule table <table>."),
    ]
    pdf.set_font("Helvetica", "B", 9)
    pdf.cell(30, 6, "Command", border=1)
    pdf.cell(38, 6, "Arguments", border=1)
    pdf.cell(102, 6, "Description", border=1, new_x="LMARGIN", new_y="NEXT")
    pdf.set_font("Helvetica", "", 8)
    fill = False
    for cmd, args, desc in commands:
        pdf.set_fill_color(240, 245, 255) if fill else pdf.set_fill_color(252, 252, 252)
        pdf.cell(30, 5.5, cmd, border=1, fill=True)
        pdf.cell(38, 5.5, args, border=1, fill=True)
        pdf.cell(102, 5.5, desc, border=1, fill=True, new_x="LMARGIN", new_y="NEXT")
        fill = not fill
    pdf.set_fill_color(255, 255, 255)
    pdf.ln(4)
    pdf.h2("Sample Session")
    pdf.code_block([
        "> help",
        "  tasks      -- show registered task list",
        "  stats      -- FreeRTOS run-time stats",
        "  pool       -- memory pool diagnostics",
        "  lat        -- latency histogram",
        "  lat reset  -- reset histogram",
        "  alarm      -- arm AUTOSAR alarm",
        "  sched      -- start schedule table",
        "  reset      -- software reset",
        "",
        "> lat",
        "=== Latency Histogram (us) ===",
        "Bucket  Range(us)   Count",
        "     0   0 -   9:     891",
        "     1  10 -  19:     104",
        "     2  20 -  29:      15",
        "Overflow (>=160us):    0",
        "Total: 1010   Mean: 3 us",
        "",
        "> tasks",
        "NAME              PRIO  HWM   WD",
        "vLatencyTask         5  210   OK",
        "vCLI_Task            3  180   OK",
        "vAutosar_OsTask      4  192   OK",
        "vDeadlockTask_A      2  130   --",
        "vDeadlockTask_B      2  130   --",
    ])


def section_contributing(pdf: DocPDF):
    pdf.add_page()
    pdf.h1("14. Contributing")
    pdf.body(
        "Contributions are welcome. Please read CONTRIBUTING.md in the "
        "repository root before submitting a pull request."
    )
    pdf.h2("Development Workflow")
    pdf.code_block([
        "# Fork the repo, then:",
        "git clone https://github.com/YOUR_USERNAME/rtos-task-manager.git",
        "cd rtos-task-manager",
        "git checkout -b feature/my-feature",
        "",
        "# Make changes, then build + test:",
        "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake",
        "cmake --build build -j$(nproc)",
        "bash scripts/run_qemu.sh  # smoke test in QEMU",
        "",
        "git add -p    # stage selectively",
        "git commit -m \"feat: describe your change\"",
        "git push origin feature/my-feature",
        "# Open a Pull Request on GitHub",
    ])
    pdf.h2("Commit Message Convention")
    pdf.kv_table([
        ("feat:", "New feature"),
        ("fix:", "Bug fix"),
        ("docs:", "Documentation only"),
        ("refactor:", "Code restructuring without behaviour change"),
        ("test:", "Adding or improving tests"),
        ("chore:", "Build, CI, tooling changes"),
        ("perf:", "Performance improvement"),
    ])
    pdf.h2("Coding Standards")
    standards = [
        "C11 standard; compile with -Wall -Wextra -Werror (no suppressions).",
        "Follow MISRA C:2012 advisory guidance where practical.",
        "All public functions must validate NULL pointer inputs.",
        "Use stdint.h types (uint8_t, uint32_t) -- avoid bare int for hardware.",
        "Name FreeRTOS task functions vXxxTask (lowercase v prefix).",
        "Add Doxygen-style comments to all new public API declarations.",
        "Keep functions under 60 lines; split if longer.",
        "No dynamic memory (malloc/free) outside mem_pool.",
    ]
    for s in standards:
        pdf.bullet(s)
    pdf.ln(2)
    pdf.h2("Pull Request Checklist")
    checks = [
        "Build passes: cmake --build build with no warnings.",
        "Smoke test passes in QEMU (run_qemu.sh exits cleanly).",
        "New public APIs have header documentation.",
        "Commit messages follow the convention above.",
        "CONTRIBUTING.md has been read and followed.",
    ]
    for c in checks:
        pdf.bullet(c)
    pdf.ln(4)
    pdf.h2("Licence")
    pdf.body(
        "This project is released under the MIT Licence. See LICENSE in the "
        "repository root. FreeRTOS is licensed under the MIT Licence by "
        "Amazon.com, Inc. or its affiliates."
    )


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    os.makedirs(DOCS_DIR, exist_ok=True)
    print(f"Generating PDF: {os.path.abspath(OUTPUT_PDF)}")

    pdf = DocPDF()
    pdf.set_title("RTOS Task Manager -- Project Documentation")
    pdf.set_author("RTOS Task Manager Project")
    pdf.set_subject("FreeRTOS ARM Cortex-M4 RTOS Task Manager")
    pdf.set_keywords("FreeRTOS RTOS ARM Cortex-M4 QEMU task manager")

    cover_page(pdf)
    toc_page(pdf)
    section_overview(pdf)
    section_architecture(pdf)
    section_task_manager(pdf)
    section_mutex_guard(pdf)
    section_mem_pool(pdf)
    section_uart_cli(pdf)
    section_autosar_os(pdf)
    section_latency_profiler(pdf)
    section_deadlock_stress(pdf)
    section_build_system(pdf)
    section_bsp(pdf)
    section_getting_started(pdf)
    section_cli_reference(pdf)
    section_contributing(pdf)

    pdf.output(OUTPUT_PDF)
    size_kb = os.path.getsize(OUTPUT_PDF) // 1024
    print(f"Done! {size_kb} KB -> {OUTPUT_PDF}")


if __name__ == "__main__":
    main()
