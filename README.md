# 🧠 RTOS Task Manager

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ARM%20Cortex--M4-blue)](https://www.arm.com/products/silicon-ip-cpu/cortex-m/cortex-m4)
[![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)](https://www.freertos.org/)
[![Standard](https://img.shields.io/badge/Standard-AUTOSAR%20OS-orange)](https://www.autosar.org/)
[![Build](https://github.com/pawanct08/rtos-task-manager/actions/workflows/ci.yml/badge.svg)](https://github.com/pawanct08/rtos-task-manager/actions/workflows/ci.yml)

A production-grade **priority-based preemptive task manager** built on FreeRTOS for ARM Cortex-M4, featuring AUTOSAR OS abstraction, deadlock detection, fixed-size memory pool allocation, and an interactive UART CLI — all runnable on **QEMU** without real hardware.

---

## ✨ Features

| Module | Description |
|---|---|
| **Task Manager** | Priority-based scheduling (Rate Monotonic), deadline miss tracking, task state machine |
| **Mutex Guard** | Deadlock detection via wait-for graph (DFS cycle detection) |
| **Memory Pool** | O(1) fixed-block allocator with zero heap fragmentation |
| **AUTOSAR OS Layer** | Maps FreeRTOS primitives to AUTOSAR Basic/Extended tasks, ISR Cat1/Cat2, Alarms |
| **UART CLI** | Runtime inspection over serial: `tasks`, `mutexes`, `pools`, `autosar`, `reset` |

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────┐
│                   Application Layer                  │
│          main.c  ·  CLI_Task  ·  User Tasks          │
├──────────────┬──────────────┬───────────────────────┤
│ task_manager │ mutex_guard  │      autosar_os        │
│──────────────│──────────────│───────────────────────│
│  mem_pool    │  uart_cli    │   FreeRTOS Kernel      │
├──────────────┴──────────────┴───────────────────────┤
│              ARM Cortex-M4 / QEMU LM3S6965EVB        │
└─────────────────────────────────────────────────────┘
```

### Task State Machine
```
  READY ──► RUNNING ──► BLOCKED ──► READY
                    └──► SUSPENDED
                    └──► DELETED
```

### Rate Monotonic Scheduling
Tasks are assigned priorities inversely proportional to their periods:

| Task | Period | Priority |
|------|--------|----------|
| High | 20 ms  | `configMAX_PRIORITIES - 1` |
| Med  | 50 ms  | `configMAX_PRIORITIES - 2` |
| Low  | 100 ms | `configMAX_PRIORITIES - 3` |

RMS schedulability is validated at startup:  
**U = Σ(Ci/Ti) ≤ n(2^(1/n) − 1)**

---

## 📁 Project Structure

```
rtos-task-manager/
├── include/
│   ├── task_manager.h    # Preemptive task manager API
│   ├── mutex_guard.h     # Deadlock-detecting mutex wrapper
│   ├── mem_pool.h        # O(1) fixed-block memory allocator
│   ├── uart_cli.h        # UART CLI task
│   └── autosar_os.h      # AUTOSAR OS abstraction layer
├── src/
│   ├── task_manager.c
│   ├── mutex_guard.c
│   ├── mem_pool.c
│   ├── uart_cli.c        # (planned)
│   └── autosar_os.c      # (planned)
├── bsp/
│   ├── startup.c         # ARM Cortex-M4 startup
│   └── lm3s6965evb.ld    # Linker script for QEMU target
├── FreeRTOS/             # Cloned by setup.sh
├── scripts/
│   ├── setup.sh          # Clone FreeRTOS & set up toolchain
│   └── run_qemu.sh       # Launch QEMU emulation
├── CMakeLists.txt
├── .github/
│   ├── workflows/ci.yml  # GitHub Actions CI
│   └── ISSUE_TEMPLATE/   # Bug & feature request templates
├── .gitignore
├── LICENSE
├── CONTRIBUTING.md
└── README.md
```

---

## 🚀 Getting Started

### Prerequisites

- `arm-none-eabi-gcc` (ARM cross-compiler)
- `cmake` ≥ 3.20
- `qemu-system-arm`
- `git`

On Ubuntu/Debian:
```bash
sudo apt install gcc-arm-none-eabi cmake qemu-system-arm git
```

### Build

```bash
# 1. Clone the repo
git clone https://github.com/pawanct08/rtos-task-manager.git
cd rtos-task-manager

# 2. Set up FreeRTOS dependency
bash scripts/setup.sh

# 3. Configure & build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build

# 4. Run on QEMU
bash scripts/run_qemu.sh
```

### UART CLI (once running in QEMU)

Connect to the emulated UART (e.g. via `telnet localhost 4321`):

```
> tasks      # Print task status table with states, periods, deadline misses
> mutexes    # Show mutex ownership & deadlock detection status
> pools      # Memory pool utilisation statistics
> autosar    # AUTOSAR OS category mapping table
> help       # List all commands
> reset      # Software reset
```

---

## 🔬 Module Deep-Dives

### Task Manager (`task_manager.h`)

- Maintains up to **8 managed tasks** alongside FreeRTOS TCBs
- Tracks: priority, period, deadline, WCET, exec count, deadline misses
- `TaskManager_Refresh()` drives the state machine — call from a monitor task
- `TaskManager_PrintStatus()` dumps a formatted table over UART

### Mutex Guard (`mutex_guard.h`)

- Wraps FreeRTOS semaphores with **wait-for graph** tracking
- On every `MutexGuard_Take()`, a DFS cycle check runs
- Deadlock detected → logs offending tasks, optionally triggers recovery
- Up to **8 tracked mutexes** across **8 tasks**

### Memory Pool (`mem_pool.h`)

- Intrusive free-list — zero extra metadata storage
- **O(1)** alloc & free, ISR-safe via FreeRTOS critical sections
- `MEM_POOL_SIZE(block_sz, num_blocks)` macro for compile-time sizing
- CLI-accessible stats: total blocks, free, allocated, OOM count

### AUTOSAR OS Layer (`autosar_os.h`)

| AUTOSAR Concept | FreeRTOS Mapping |
|---|---|
| Basic Task | FreeRTOS task, no event wait |
| Extended Task | FreeRTOS task + EventGroup |
| ISR Category 1 | Direct vector handler |
| ISR Category 2 | FromISR API handler |
| Resource | Binary Semaphore / Mutex |
| Alarm | Software Timer |
| Counter | Tick count / hardware timer |

---

## 🤝 Contributing

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute bug fixes, new features, or documentation improvements.

---

## 📄 License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.
