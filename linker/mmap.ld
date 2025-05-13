OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vector_table)

_PUBLIC_RAM_INIT        = 0x70010000;
_STACK_INIT             = 0x70020000;
_MMU_INIT        	    = 0x70080000;

_SYS_STACK_SIZE         = 4K;
_ABT_STACK_SIZE         = 4K;
_SVC_STACK_SIZE         = 4K;
_IRQ_STACK_SIZE         = 4K;
_UND_STACK_SIZE         = 4K;

_STACK_SIZE             = _ABT_STACK_SIZE + _SVC_STACK_SIZE + _IRQ_STACK_SIZE + _UND_STACK_SIZE + _SYS_STACK_SIZE;
_TASK_STACK_SIZE        = 4K;
_TOTAL_STACK_SIZE       = _STACK_SIZE + 8 * _TASK_STACK_SIZE;

/* Has to match the kernel/inc/sched.h define statement */
MAX_TASKS               = 4;
_PAGE_SIZE_L1       	= 16K;
_PAGE_SIZE_L2       	= 1K;
_TOTAL_MMU_REGION_SIZE  = (_PAGE_SIZE_L1 + 8 * 2 * _PAGE_SIZE_L2) * MAX_TASKS;

MEMORY {
    PUBLIC_RAM : ORIGIN = _PUBLIC_RAM_INIT, LENGTH = 16K
    PUBLIC_STACK : ORIGIN = _STACK_INIT, LENGTH = _TOTAL_STACK_SIZE /* 20K + 8 * 4K */
    MMU_REGION : ORIGIN = _MMU_INIT, LENGTH = _TOTAL_MMU_REGION_SIZE /* (16K + 2 * 8K) * 4 */
}

SECTIONS {
    .text : {
        . = ALIGN(4);
        KEEP (*(.vector_table))
        *(.text*)
    } > PUBLIC_RAM

    .data : {
        *(.data*)
    } > PUBLIC_RAM

    .bss : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        __bss_end__ = .;
    } > PUBLIC_RAM

	.tables : {
    	/* The alignment is for the table size */
    	. = ALIGN(16K);
        __mmu_tables_start__ = .;
        *(.mmu_tables*)
        __mmu_tables_end__ = .;
	} > MMU_REGION

    /* 16-byte alignment is sometimes used to ensure compatibility
    with SIMD (Single Instruction, Multiple Data) instructions,
    such as those found in ARM NEON or Intel SSE/AVX,
    which often require 16-byte alignment for their data. */
    .stack (NOLOAD) : {
        . = ALIGN(16);
        __stack_end = .;

        /* 0x70020000 */
        _sys_stack_end = .;
        . += _SYS_STACK_SIZE;
        . = ALIGN(16);
        __sys_sp = .;
        /* 0x70021000 */

        _irq_stack_end = .;
        . += _IRQ_STACK_SIZE;
        . = ALIGN(16);
        __irq_sp = .;
        /* 0x70022000 */

        _svc_stack_end = .;
        . += _SVC_STACK_SIZE;
        . = ALIGN(16);
        __svc_sp = .;
        /* 0x70023000 */

        _abt_stack_end = .;
        . += _ABT_STACK_SIZE;
        . = ALIGN(16);
        __abt_sp = .;
        /* 0x70024000 */

        _und_stack_end = .;
        . += _UND_STACK_SIZE;
        . = ALIGN(16);
        __und_sp = .;
        /* 0x70025000 */

        _task0_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task0_sp = .;
        /* 0x70026000 */

        _task1_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task1_sp = .;
        /* 0x70027000 */

        _task2_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task2_sp = .;
        /* 0x70028000 */

        _task3_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task3_sp = .;
        /* 0x70029000 */

        /* IRQ_SP for tasks */
        _task0_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task0_irq_sp = .;
        /* 0x7002A000 */

        _task1_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task1_irq_sp = .;
        /* 0x7002B000 */

        _task2_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task2_irq_sp = .;
        /* 0x7002C0000 */

        _task3_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task3_irq_sp = .;
        /* 0x7002D000 */

        __stack_start = .;
    } > PUBLIC_STACK
}
