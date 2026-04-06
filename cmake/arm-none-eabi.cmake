set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Use full path so cmake --regenerate-during-build (via cmd.exe / system PATH)
# always finds the 14.2 toolchain instead of yagarto on the system PATH.
set(_TC_BIN "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin")
set(CMAKE_C_COMPILER   "${_TC_BIN}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${_TC_BIN}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${_TC_BIN}/arm-none-eabi-gcc.exe")
set(CMAKE_OBJCOPY      "${_TC_BIN}/arm-none-eabi-objcopy.exe")
set(CMAKE_SIZE         "${_TC_BIN}/arm-none-eabi-size.exe")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
