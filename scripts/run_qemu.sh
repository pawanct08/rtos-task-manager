#!/bin/bash
set -e

# Run QEMU with the generated ELF file
# Target: mps2-an386 (ARM Cortex-M4 with FPU) — matches our CPU flags
# UART output appears directly in this terminal via -serial stdio
# Press Ctrl+A then X to quit QEMU

ELF_FILE="build/rtos-task-manager.elf"

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: $ELF_FILE not found. Please build first:"
    echo "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake"
    echo "  cmake --build build"
    exit 1
fi

if ! command -v qemu-system-arm &> /dev/null; then
    echo "Error: qemu-system-arm not installed."
    echo "  sudo apt install qemu-system-arm"
    exit 1
fi

echo "========================================"
echo "  RTOS Task Manager — QEMU mps2-an386"
echo "  Ctrl+A then X to quit"
echo "========================================"
echo ""

qemu-system-arm \
    -machine mps2-an386 \
    -cpu cortex-m4 \
    -kernel "$ELF_FILE" \
    -serial stdio \
    -nographic \
    -semihosting-config enable=on,target=native
