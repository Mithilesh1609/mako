
    KERNEL_START_VADDR equ 0xC0000000
    KERNEL_PD_IDX      equ (KERNEL_START_VADDR >> 22)
    KERNEL_STACK_SIZE  equ 0x1000