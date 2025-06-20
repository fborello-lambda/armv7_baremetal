OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_vector_table)

/* Kernel .bss */
_KERNEL_BSS_PHY         = 0x70022000;
_KERNEL_BSS_VMA         = 0x70022000;
/* Kernel .rodata */
_KERNEL_RODATA_PHY      = 0x70023000;
_KERNEL_RODATA_VMA      = 0x70023000;
/* Kernel .text */
_KERNEL_TEXT_PHY        = 0x70030000;
_KERNEL_TEXT_VMA        = 0x70030000;
/* Kernel .data */
_KERNEL_DATA_PHY        = 0x70031000;
_KERNEL_DATA_VMA        = 0x70031000;

/* .task0.text */
_TASK0_TEXT_PHY         = 0x70F60000;
_TASK0_TEXT_VMA         = 0x70F60000;

/* .task1.text */
_TASK1_TEXT_PHY         = 0x80750000;
_TASK1_TEXT_VMA         = 0x70F50000;
/* .task1.data */
_TASK1_DATA_PHY         = 0x80751000;
_TASK1_DATA_VMA         = 0x70F51000;
/* TASK1 Stack */
_TASK1_STACK_PHY        = 0x80752000;
_TASK1_STACK            = 0x70F52000;
/* .task1.bss */
_TASK1_BSS_PHY          = 0x80753000;
_TASK1_BSS_VMA          = 0x70F53000;
/* .task1.rodata */
_TASK1_RODATA_PHY       = 0x80754000;
_TASK1_RODATA_VMA       = 0x70F54000;
/* TASK1 reading area */
_TASK1_RAREA_START_PHY  = 0x80000000;
_TASK1_RAREA_START_VMA  = 0x70A00000;
_TASK1_RAREA_END_VMA    = 0x70A0FFFF;

/* .task2.text */
_TASK2_TEXT_PHY         = 0x80740000;
_TASK2_TEXT_VMA         = 0x70F40000;
/* .task2.data */
_TASK2_DATA_PHY         = 0x80741000;
_TASK2_DATA_VMA         = 0x70F41000;
/* .task2.bss */
_TASK2_BSS_PHY          = 0x80743000;
_TASK2_BSS_VMA          = 0x70F43000;
/* .task2.rodata */
_TASK2_RODATA_PHY       = 0x80744000;
_TASK2_RODATA_VMA       = 0x70F44000;
/* TASK2 Stack */
_TASK2_STACK_PHY        = 0x80742000;
_TASK2_STACK            = 0x70F42000;
/* TASK2 reading area */
_TASK2_RAREA_START_PHY  = 0x80010000;
_TASK2_RAREA_START_VMA  = 0x70A10000;
_TASK2_RAREA_END_VMA    = 0x70A1FFFF;

_PUBLIC_RAM_INIT        = 0x70010000;
_KERNEL_STACK           = 0x70020000;
_MMU_INIT        	    = 0x70080000;

_SYS_STACK_SIZE         = 1K;
_ABT_STACK_SIZE         = 1K;
_SVC_STACK_SIZE         = 512;
_IRQ_STACK_SIZE         = 1K;
_UND_STACK_SIZE         = 512;

_STACK_SIZE             = _ABT_STACK_SIZE + _SVC_STACK_SIZE + _IRQ_STACK_SIZE + _UND_STACK_SIZE + _SYS_STACK_SIZE;
_TASK_STACK_SIZE        = 1K;

/* Has to match the kernel/inc/sched.h define statement */
MAX_TASKS               = 3;
_PAGE_SIZE_L1       	= 16K;
_PAGE_SIZE_L2       	= 1K;
_TOTAL_MMU_REGION_SIZE  = (_PAGE_SIZE_L1 + 8 * 2 * _PAGE_SIZE_L2) * MAX_TASKS;

_KERNEL_STACK_SIZE      = _STACK_SIZE + 4 * _TASK_STACK_SIZE;
_TASK1_STACK_SIZE       = _TASK_STACK_SIZE * 4;
_TASK2_STACK_SIZE       = _TASK_STACK_SIZE * 4;

MEMORY {
    PUBLIC_RAM      : ORIGIN    = _PUBLIC_RAM_INIT, LENGTH = 32M
    PUBLIC_STACK    : ORIGIN    = _KERNEL_STACK,    LENGTH = _STACK_SIZE + 4 * _TASK_STACK_SIZE
    MMU_REGION      : ORIGIN    = _MMU_INIT,        LENGTH = _TOTAL_MMU_REGION_SIZE
    TASK1_STACK     : ORIGIN    = _TASK1_STACK,     LENGTH = _TASK1_STACK_SIZE
    TASK2_STACK     : ORIGIN    = _TASK2_STACK,     LENGTH = _TASK2_STACK_SIZE
}

SECTIONS {
    .text : {
        . = ALIGN(4);
        KEEP (*(.vector_table))
        *(.text*)
    } > PUBLIC_RAM

    .data : {
        . = ALIGN(4);
        *(.data*)
    } > PUBLIC_RAM

    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        __bss_end__ = .;
    } > PUBLIC_RAM

	.tables (NOLOAD) : {
    	/* The alignment is for the table size */
    	. = ALIGN(16K);
        __mmu_tables_start__ = .;
        *(.mmu_tables*)
        __mmu_tables_end__ = .;
	} > MMU_REGION

    /* --- Kernel sections --- */
    _KERNEL_TEXT_LMA = .;
    .kernel.text _KERNEL_TEXT_VMA : AT(_KERNEL_TEXT_LMA) {
        . = ALIGN(4);
        *(.kernel.text*)
    } > PUBLIC_RAM
    _KERNEL_TEXT_SIZE = SIZEOF(.kernel.text);

    _KERNEL_DATA_LMA = _KERNEL_TEXT_LMA + _KERNEL_TEXT_SIZE;
    .kernel.data _KERNEL_DATA_VMA : AT(_KERNEL_DATA_LMA) {
        . = ALIGN(4);
        *(.kernel.data*)
    } > PUBLIC_RAM
    _KERNEL_DATA_SIZE = SIZEOF(.kernel.data);

    _KERNEL_BSS_LMA = _KERNEL_DATA_LMA + _KERNEL_DATA_SIZE;
    .kernel.bss _KERNEL_BSS_VMA (NOLOAD) : {
        *(.kernel.bss*)
    } > PUBLIC_RAM
    _KERNEL_BSS_SIZE = SIZEOF(.kernel.bss);

    _KERNEL_RODATA_LMA = _KERNEL_BSS_LMA + _KERNEL_BSS_SIZE;
    .kernel.rodata _KERNEL_RODATA_VMA : AT(_KERNEL_RODATA_LMA) {
        . = ALIGN(4);
        *(.kernel.rodata*)
        *(.rodata)
    } > PUBLIC_RAM
    _KERNEL_RODATA_SIZE = SIZEOF(.kernel.rodata);

    /* --- Task 0 --- */
    _TASK0_TEXT_LMA = _KERNEL_RODATA_LMA + _KERNEL_RODATA_SIZE;
    .task0.text _TASK0_TEXT_VMA : AT(_TASK0_TEXT_LMA) {
        . = ALIGN(4);
        *(.task0.text*)
    } > PUBLIC_RAM
    _TASK0_TEXT_SIZE = SIZEOF(.task0.text);

    /* --- Task 1 --- */
    _TASK1_TEXT_LMA = _TASK0_TEXT_LMA + _TASK0_TEXT_SIZE;
    .task1.text _TASK1_TEXT_VMA : AT(_TASK1_TEXT_LMA) {
        . = ALIGN(4);
        *(.task1.text*)
    } > PUBLIC_RAM
    _TASK1_TEXT_SIZE = SIZEOF(.task1.text);

    _TASK1_DATA_LMA = _TASK1_TEXT_LMA + _TASK1_TEXT_SIZE;
    .task1.data _TASK1_DATA_VMA : AT(_TASK1_DATA_LMA) {
        . = ALIGN(4);
        *(.task1.data*)
    } > PUBLIC_RAM
    _TASK1_DATA_SIZE = SIZEOF(.task1.data);

    _TASK1_BSS_LMA = _TASK1_DATA_LMA + _TASK1_DATA_SIZE;
    .task1.bss _TASK1_BSS_VMA (NOLOAD) : {
        . = ALIGN(4);
        *(.task1.bss*)
    } > PUBLIC_RAM
    _TASK1_BSS_SIZE = SIZEOF(.task1.bss);

    _TASK1_RODATA_LMA = _TASK1_BSS_LMA + _TASK1_BSS_SIZE;
    .task1.rodata _TASK1_RODATA_VMA : AT(_TASK1_RODATA_LMA) {
        . = ALIGN(4);
        *(.task1.rodata*)
    } > PUBLIC_RAM
    _TASK1_RODATA_SIZE = SIZEOF(.task1.rodata);

    /* --- Task 2 --- */
    _TASK2_TEXT_LMA = _TASK1_RODATA_LMA + _TASK1_RODATA_SIZE;
    .task2.text _TASK2_TEXT_VMA : AT(_TASK2_TEXT_LMA) {
        . = ALIGN(4);
        *(.task2.text*)
    } > PUBLIC_RAM
    _TASK2_TEXT_SIZE = SIZEOF(.task2.text);

    _TASK2_DATA_LMA = _TASK2_TEXT_LMA + _TASK2_TEXT_SIZE;
    .task2.data _TASK2_DATA_VMA : AT(_TASK2_DATA_LMA) {
        . = ALIGN(4);
        *(.task2.data*)
    } > PUBLIC_RAM
    _TASK2_DATA_SIZE = SIZEOF(.task2.data);

    _TASK2_BSS_LMA = _TASK2_DATA_LMA + _TASK2_DATA_SIZE;
    .task2.bss _TASK2_BSS_VMA (NOLOAD) : {
        . = ALIGN(4);
        *(.task2.bss*)
    } > PUBLIC_RAM
    _TASK2_BSS_SIZE = SIZEOF(.task2.bss);

    _TASK2_RODATA_LMA = _TASK2_BSS_LMA + _TASK2_BSS_SIZE;
    .task2.rodata _TASK2_RODATA_VMA : AT(_TASK2_RODATA_LMA) {
        . = ALIGN(4);
        *(.task2.rodata*)
    } > PUBLIC_RAM
    _TASK2_RODATA_SIZE = SIZEOF(.task2.rodata);

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

        /* 0x70020400 */
        _irq_stack_end = .;
        . += _IRQ_STACK_SIZE;
        . = ALIGN(16);
        __irq_sp = .;

        /* 0x70020800 */
        _svc_stack_end = .;
        . += _SVC_STACK_SIZE;
        . = ALIGN(16);
        __svc_sp = .;

        /* 0x70020A00 */
        _abt_stack_end = .;
        . += _ABT_STACK_SIZE;
        . = ALIGN(16);
        __abt_sp = .;

        /* 0x70020E00 */
        _und_stack_end = .;
        . += _UND_STACK_SIZE;
        . = ALIGN(16);
        __und_sp = .;

        /* 0x70021000 */
        /* SP for task0 */
        _task0_stack_end = .;
        . += (_TASK_STACK_SIZE);
        . = ALIGN(16);
        __task0_sp = .;
        
        /* 0x70021400 */
        /* IRQ_SP for task0 */
        _task0_irq_end = .;
        . += (_TASK_STACK_SIZE);
        . = ALIGN(16);
        __task0_irq_sp = .;

        /* 0x70021800 */
        __stack_start = .;
    } > PUBLIC_STACK

    .task1.stack (NOLOAD) : {
        /* 0x70F52000 */
        _task1_stack_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task1_sp = .;
        /* 0x70F52400*/
         _task1_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        /* 0x70F52800*/
        __task1_irq_sp = .;
    } > TASK1_STACK

    .task2.stack (NOLOAD) : {
        /* 0x70F42000 */
        _task2_stack_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task2_sp = .;
        /* 0x70F42400 */
        _task2_irq_end = .;
        . += _TASK_STACK_SIZE;
        . = ALIGN(16);
        __task2_irq_sp = .;
        /* 0x70F42800 */
    } > TASK2_STACK
}
