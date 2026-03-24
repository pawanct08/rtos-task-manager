#!/bin/bash
set -e

# Setup script for FreeRTOS RTOS Task Manager

FREERTOS_VERSION="V10.5.1"
FREERTOS_DIR="FreeRTOS"

echo "Setting up RTOS Task Manager workspace..."

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo "Error: git is not installed."
    exit 1
fi

# Clone FreeRTOS if not already present
if [ ! -d "$FREERTOS_DIR" ]; then
    echo "Cloning FreeRTOS $FREERTOS_VERSION..."
    git clone -b $FREERTOS_VERSION --depth 1 https://github.com/FreeRTOS/FreeRTOS.git $FREERTOS_DIR
else
    echo "FreeRTOS already cloned in ./$FREERTOS_DIR"
fi

# We specifically need FreeRTOS/Source and portable files
if [ ! -d "$FREERTOS_DIR/FreeRTOS/Source/portable/GCC/ARM_CM4F" ]; then
    echo "Error: FreeRTOS ARM CM4F portable directory not found!"
    exit 1
fi

echo "Setup complete! You can now build the project with CMake."
