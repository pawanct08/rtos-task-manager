set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

if(CMAKE_HOST_WIN32)
    # Windows: use absolute path to avoid picking up stale toolchains on PATH
    set(_TC_BIN "C:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.2 rel1/bin")
    set(CMAKE_C_COMPILER   "${_TC_BIN}/arm-none-eabi-gcc.exe")
    set(CMAKE_CXX_COMPILER "${_TC_BIN}/arm-none-eabi-g++.exe")
    set(CMAKE_ASM_COMPILER "${_TC_BIN}/arm-none-eabi-gcc.exe")
    set(CMAKE_OBJCOPY      "${_TC_BIN}/arm-none-eabi-objcopy.exe")
    set(CMAKE_SIZE         "${_TC_BIN}/arm-none-eabi-size.exe")
else()
    # Linux/macOS (CI): compiler installed to PATH via apt/brew
    set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
    set(CMAKE_SIZE         arm-none-eabi-size)
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
