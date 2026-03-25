#!/bin/bash
# scripts/debug_qemu.sh — GDB server mode on mps2-an386
# Terminal 1: ./scripts/debug_qemu.sh
# Terminal 2: arm-none-eabi-gdb build/rtos-task-manager.elf
#             (gdb) target remote :3333
#             (gdb) load && b main && c

ELF_FILE="build/rtos-task-manager.elf"
if [ ! -f "$ELF_FILE" ]; then
    echo "Build first:"
    echo "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake"
    echo "  cmake --build build"
    exit 1
fi

echo "QEMU GDB server on :3333 (waiting for connection)..."
qemu-system-arm \
    -machine mps2-an386 \
    -cpu cortex-m4 \
    -kernel "$ELF_FILE" \
    -serial stdio \
    -nographic \
    -S -gdb tcp::3333 \
    -semihosting-config enable=on,target=native
