#!/bin/bash
set -e

# Setup script — installs toolchain + clones FreeRTOS
# Run once after cloning: chmod +x scripts/setup.sh && ./scripts/setup.sh

FREERTOS_VERSION="V10.5.1"
FREERTOS_DIR="FreeRTOS"

echo "========================================"
echo "  RTOS Task Manager — Setup"
echo "========================================"

# ── System dependencies ──────────────────────────────────────────────────────
if command -v apt-get &>/dev/null; then
    echo "==> Installing system dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y \
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        qemu-system-arm \
        cmake ninja-build \
        git \
        python3-pip
else
    echo "Note: auto-install only supported on Debian/Ubuntu."
    echo "      Install manually: gcc-arm-none-eabi, qemu-system-arm, cmake, git"
fi

# ── Python plotting dependencies ─────────────────────────────────────────────
echo "==> Installing Python dependencies..."
pip3 install --quiet matplotlib numpy

# ── FreeRTOS ─────────────────────────────────────────────────────────────────
if ! command -v git &>/dev/null; then
    echo "Error: git is not installed."
    exit 1
fi

if [ ! -d "$FREERTOS_DIR" ]; then
    echo "==> Cloning FreeRTOS $FREERTOS_VERSION..."
    git clone -b $FREERTOS_VERSION --depth 1 \
        https://github.com/FreeRTOS/FreeRTOS.git $FREERTOS_DIR
else
    echo "==> FreeRTOS already present in ./$FREERTOS_DIR — skipping."
fi

if [ ! -d "$FREERTOS_DIR/FreeRTOS/Source/portable/GCC/ARM_CM4F" ]; then
    echo "Error: FreeRTOS ARM CM4F portable directory not found!"
    echo "       Try: rm -rf FreeRTOS && ./scripts/setup.sh"
    exit 1
fi

# ── Verify ───────────────────────────────────────────────────────────────────
echo ""
echo "==> Toolchain versions:"
arm-none-eabi-gcc --version | head -1
qemu-system-arm --version | head -1
cmake --version | head -1

echo ""
echo "========================================"
echo "  Setup complete!"
echo ""
echo "  Build:"
echo "    cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake"
echo "    cmake --build build"
echo ""
echo "  Run:"
echo "    ./scripts/run_qemu.sh"
echo ""
echo "  Debug:"
echo "    ./scripts/debug_qemu.sh   (then GDB in another terminal)"
echo "========================================"
