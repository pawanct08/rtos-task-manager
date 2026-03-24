cmake_minimum_required(VERSION 3.20)

project(rtos-task-manager C ASM)

# ── Target: ARM Cortex-M4 ────────────────────────────────────────────────────
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Use arm-none-eabi toolchain (override via -DCMAKE_TOOLCHAIN_FILE if needed)
set(CMAKE_C_COMPILER    arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_OBJCOPY       arm-none-eabi-objcopy)
set(CMAKE_SIZE          arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── CPU Flags ────────────────────────────────────────────────────────────────
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS   "${CPU_FLAGS} -Wall -Wextra -Os -ffunction-sections -fdata-sections -std=c11")
set(CMAKE_EXE_LINKER_FLAGS
    "${CPU_FLAGS} -T${CMAKE_SOURCE_DIR}/bsp/lm3s6965evb.ld \
     -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map --specs=nosys.specs --specs=nano.specs")

# ── FreeRTOS paths (cloned by scripts/setup.sh) ──────────────────────────────
set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source)

set(FREERTOS_SOURCES
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

# ── Application Sources ───────────────────────────────────────────────────────
set(APP_SOURCES
    src/task_manager.c
    src/mutex_guard.c
    src/mem_pool.c
    src/uart_cli.c
    src/autosar_os.c
    src/main.c
    bsp/startup.c
)

# ── Include Directories ───────────────────────────────────────────────────────
include_directories(
    include/
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/include
)

# ── Build Target ──────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${FREERTOS_SOURCES}
)

# ── Post-build: generate .bin and print size ──────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE}   --format=berkeley ${PROJECT_NAME}.elf
    COMMENT "Generating ${PROJECT_NAME}.bin"
)
