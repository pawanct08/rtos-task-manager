/*
 * mps2_an386.ld  —  Linker script for QEMU mps2-an386 (ARM Cortex-M4)
 *
 * Memory map (from MPS2+ AN386 TRM):
 *   ZBT SSRAM1 (code)  : 0x00000000, 4MB   — used for FLASH (read-execute)
 *   ZBT SSRAM2/3       : 0x20000000, 4MB   — used for SRAM  (read-write)
 *
 * This replaces lm3s6965evb.ld which was a Cortex-M3 target.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4096K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 4096K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    /*--- Vector table + code ---*/
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /*--- Initialised data (load from FLASH, run in SRAM) ---*/
    .data :
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH

    _sidata = LOADADDR(.data);

    /*--- Zero-init data ---*/
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* _sbrk heap start — required by newlib nosys */
    end = _ebss;

    /*--- FreeRTOS heap (heap_4) + stack headroom ---*/
    .heap :
    {
        . = ALIGN(8);
        _heap_start = .;
        . += 0xC800;    /* 50 KB — matches configTOTAL_HEAP_SIZE */
        _heap_end = .;
    } > SRAM

    /* Discard C++ exception tables (not used in bare-metal C) */
    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
