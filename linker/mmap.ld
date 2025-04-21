OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vector_table)

_PUBLIC_RAM_INIT        = 0x70010000;
_STACK_INIT             = 0x70020000;
/* Define a region to start the translation tables used by the MMU */
_MMU_INIT        	    = 0x70080000;

_SYS_STACK_SIZE         = 8K;
_ABT_STACK_SIZE         = 1K;
_SVC_STACK_SIZE         = 2K;
_IRQ_STACK_SIZE         = 1K;
_UND_STACK_SIZE         = 1K;

_STACK_SIZE             = _ABT_STACK_SIZE + _SVC_STACK_SIZE + _IRQ_STACK_SIZE + _UND_STACK_SIZE + _SYS_STACK_SIZE;
_TASK_STACK_SIZE        = 1K;
_TOTAL_STACK_SIZE       = _STACK_SIZE + 8 * _TASK_STACK_SIZE;


_PAGE_SIZE_L1       	= 16K;
_PAGE_SIZE_L2       	= 1K;
_TOTAL_MMU_REGION_SIZE  = _PAGE_SIZE_L1 + 4 * _PAGE_SIZE_L2;

MEMORY {
    PUBLIC_RAM : ORIGIN = _PUBLIC_RAM_INIT, LENGTH = 16K
    PUBLIC_STACK : ORIGIN = _STACK_INIT, LENGTH = _TOTAL_STACK_SIZE /* 21K */
    MMU_REGION : ORIGIN = _MMU_INIT, LENGTH = _TOTAL_MMU_REGION_SIZE /* 16K + 4K */
}

/* It starts from the bottom and grows upwards */
__sp = ORIGIN(PUBLIC_STACK) + LENGTH(PUBLIC_STACK);

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
    	__l1_table_start__ = .;
    	*(.l1_table*)
    	__l1_table_end__ = .;
    	__l2_tables_start__ = .;
    	*(.l2_table*)
    	__l2_tables_end__ = .;
	} > MMU_REGION

    /* 16-byte alignment is sometimes used to ensure compatibility
    with SIMD (Single Instruction, Multiple Data) instructions,
    such as those found in ARM NEON or Intel SSE/AVX,
    which often require 16-byte alignment for their data.*/
    .stack (NOLOAD) : {
        . = ALIGN(16);
        _stack_end = .;

        /* 0x70020000 */
        _sys_stack_end = .;
        . += _SYS_STACK_SIZE;
        . = ALIGN(16);
        __sys_sp = .;
        /* 0x70022000 */

        _irq_stack_end = .;
        . += _IRQ_STACK_SIZE;
        . = ALIGN(16);
        __irq_sp = .;
        /* 0x70022400 */

        _svc_stack_end = .;
        . += _SVC_STACK_SIZE;
        . = ALIGN(16);
        __svc_sp = .;
        /* 0x70022C00 */

        _abt_stack_end = .;
        . += _ABT_STACK_SIZE;
        . = ALIGN(16);
        __abt_sp = .;
        /* 0x7002300 */

        _und_stack_end = .;
        . += _UND_STACK_SIZE;
        . = ALIGN(16);
        __und_sp = .;
        /* 0x70023400 */

        _task0_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task0_sp = .;
        /* 0x70023800 */

        _task1_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task1_sp = .;
        /* 0x70023C00 */

        _task2_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task2_sp = .;
        /* 0x70024000 */

        _task3_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task3_sp = .;
        /* 0x70024400 */

        /* IRQ_SP for tasks */
        _task0_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task0_irq_sp = .;
        /* 0x70024800 */

        _task1_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task1_irq_sp = .;
        /* 0x70024C00 */

        _task2_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task2_irq_sp = .;
        /* 0x70025000 */

        _task3_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task3_irq_sp = .;
        /* 0x70025400 */

        __stack_start = .;
    } > PUBLIC_STACK
}
