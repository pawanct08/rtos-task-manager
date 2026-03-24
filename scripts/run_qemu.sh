#!/bin/bash
set -e

# Run QEMU with the generated ELF file

ELF_FILE="build/rtos-task-manager.elf"

if [ ! -f "$ELF_FILE" ]; then
    echo "Error: $ELF_FILE not found. Please build the project first."
    echo "  cmake -B build"
    echo "  cmake --build build"
    exit 1
fi

if ! command -v qemu-system-arm &> /dev/null; then
    echo "Error: qemu-system-arm not installed."
    exit 1
fi

echo "Starting QEMU. Press Ctrl+A, then X to exit."
qemu-system-arm -M lm3s6965evb -cpu cortex-m3 -kernel "$ELF_FILE" -nographic
