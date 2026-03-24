#include <stdint.h>
#include "FreeRTOSConfig.h"

extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _etext;
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

    /* Copy .data section from flash to SRAM */
    src = &_etext;
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

void Default_Handler(void) {
    while (1);
}

/* ─── Vector Table for Cortex-M3 (QEMU LM3S6965EVB) ─── */
__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))&_estack,
    Reset_Handler,
    Default_Handler, /* NMI */
    Default_Handler, /* HardFault */
    Default_Handler, /* MemManage */
    Default_Handler, /* BusFault */
    Default_Handler, /* UsageFault */
    0, 0, 0, 0,      /* Reserved */
    vPortSVCHandler, /* SVCall */
    Default_Handler, /* Debug Monitor */
    0,               /* Reserved */
    xPortPendSVHandler, /* PendSV */
    xPortSysTickHandler /* SysTick */
};
