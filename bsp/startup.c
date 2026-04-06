#include <stdint.h>
#include "FreeRTOSConfig.h"

extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sidata;   /* LOADADDR(.data) — actual .data LMA in Flash */
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

/* FreeRTOS port handlers */
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void vPortSVCHandler(void);

extern int main(void);

void Reset_Handler(void) {
    uint32_t *src, *dest;

    /* ── Enable FPU before .data/.bss init and main() ──────────────────────
     * ARM_CM4F hard-float ABI (-mfloat-abi=hard) emits FPU instructions in
     * ANY compiled function, including FreeRTOS internals called from
     * xTaskCreate().  Without this, the very first FPU instruction raises
     * UsageFault → Default_Handler → silent while(1) hang.
     * FreeRTOS xPortStartScheduler() also enables the FPU, but only after
     * vTaskStartScheduler() — too late for pre-scheduler code paths.
     * CPACR is at 0xE000ED88; bits [23:22]=CP11, [21:20]=CP10, value 0xF
     * grants full access for both coprocessors. */
    *((volatile uint32_t *)0xE000ED88U) |= (0xFU << 20U);

    /* Copy .data section from flash to SRAM.
     * Use _sidata (= LOADADDR(.data)), NOT _etext.  The linker places
     * .init and .fini sections between the end of .text (_etext) and the
     * start of .data (_sidata), creating an 8-byte gap.  Copying from
     * _etext would shift every .data initialiser by 8 bytes, corrupting
     * uxCriticalNesting and all other initialised variables. */
    src = &_sidata;
    for (dest = &_sdata; dest < &_edata;) {
        *dest++ = *src++;
    }

    /* Zero-fill .bss section */
    for (dest = &_sbss; dest < &_ebss;) {
        *dest++ = 0;
    }

    /* Branch to main */
    main();
    while (1);
}

/* -------------------------------------------------------------------------
 * Raw UART output for fault handlers.
 * CMSDK APB UART0 on mps2-an386 lives at 0x40004000.
 * bit-0 of STATE register (0x40004004) = TX buffer full.
 * Safe to call before or after UART_Init() — QEMU accepts chars
 * as soon as the peripheral is addressed.
 * -------------------------------------------------------------------------*/
#define _UART0_DATA    (*(volatile uint32_t *)0x40004000U)
#define _UART0_STATE   (*(volatile uint32_t *)0x40004004U)

static void bsp_fault_puts(const char *s)
{
    for (; *s; s++) {
        while (_UART0_STATE & 0x1U) {}   /* wait while TX buffer full */
        _UART0_DATA = (uint32_t)(unsigned char)*s;
    }
}

void HardFault_Handler(void)  { bsp_fault_puts("\r\n!!! HardFault !!!\r\n");      while (1); }
void BusFault_Handler(void)   { bsp_fault_puts("\r\n!!! BusFault !!!\r\n");       while (1); }
void UsageFault_Handler(void) { bsp_fault_puts("\r\n!!! UsageFault !!!\r\n");     while (1); }
void MemManage_Handler(void)  { bsp_fault_puts("\r\n!!! MemManageFault !!!\r\n"); while (1); }

void Default_Handler(void) {
    bsp_fault_puts("\r\n!!! Unexpected exception !!!\r\n");
    while (1);
}

/* ─── Vector Table — Cortex-M4F (QEMU mps2-an386) ─── */
__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))&_estack,
    Reset_Handler,
    Default_Handler,    /* NMI */
    HardFault_Handler,  /* HardFault */
    MemManage_Handler,  /* MemManage */
    BusFault_Handler,   /* BusFault */
    UsageFault_Handler, /* UsageFault */
    0, 0, 0, 0,         /* Reserved */
    vPortSVCHandler,    /* SVCall */
    Default_Handler,    /* Debug Monitor */
    0,                  /* Reserved */
    xPortPendSVHandler, /* PendSV */
    xPortSysTickHandler /* SysTick */
};
