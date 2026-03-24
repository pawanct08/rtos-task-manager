# RTOS Task Manager — Architecture Plan

## Directory Structure
rtos-task-manager/
├── CMakeLists.txt
├── scripts/
│   ├── setup.sh
│   └── run_qemu.sh
├── FreeRTOS/          ← cloned by setup.sh
├── include/
│   ├── task_manager.h
│   ├── mutex_guard.h
│   ├── mem_pool.h
│   ├── uart_cli.h
│   └── autosar_os.h
├── src/
│   ├── main.c
│   ├── task_manager.c
│   ├── mutex_guard.c
│   ├── mem_pool.c
│   ├── uart_cli.c
│   └── autosar_os.c
├── bsp/
│   ├── startup.c
│   └── lm3s6965evb.ld   ← linker script for QEMU target
└── README.md
