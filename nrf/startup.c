#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

extern int main(void);

void Default_Handler(void) {
    while (1);
}

void Reset_Handler(void) {
    // Enable FPU CP10/CP11
    *(volatile uint32_t *)0xE000ED88 |= (0xFUL << 20);
    __asm__ volatile ("dsb; isb");

    // Copy .data
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    // Zero .bss
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();
    while (1);
}

__attribute__((section(".isr_vector"), used))
const void* vector_table[] = {
    &_estack,                  // 0: Initial Stack Pointer
    (void*)Reset_Handler,      // 1: Reset
    (void*)Default_Handler,    // 2: NMI
    (void*)Default_Handler,    // 3: HardFault
    (void*)Default_Handler,    // 4: MemManage
    (void*)Default_Handler,    // 5: BusFault
    (void*)Default_Handler,    // 6: UsageFault
    0,                         // 7: Reserved
    0,                         // 8: Reserved
    0,                         // 9: Reserved
    0,                         // 10: Reserved
    (void*)Default_Handler,    // 11: SVCall
    (void*)Default_Handler,    // 12: Debug Monitor
    0,                         // 13: Reserved
    (void*)Default_Handler,    // 14: PendSV
    (void*)Default_Handler     // 15: SysTick
};