.global _reset_handler

.extern main
.extern _vector_table
.extern _vector_table_end

.extern __abt_sp
.extern __irq_sp
.extern __svc_sp
.extern __sys_sp
.extern __und_sp

.equ USR_MODE,  0b10000
.equ IRQ_MODE,  0b10010
.equ SVC_MODE,  0b10011
.equ ABT_MODE,  0b10111
.equ UND_MODE,  0b11011
.equ SYS_MODE,  0b11111
.equ I_Bit,     0x80    // when I bit is set, IRQ is disabled
.equ F_Bit,     0x40    // when F bit is set, FIQ is disabled

.section .text._reset_handler
_reset_handler:
    // Following:
    // https://developer.arm.com/documentation/den0013/d/ARM-Processor-Modes-and-Registers
    // https://developer.arm.com/documentation/dui0471/m/embedded-software-development/stack-pointer-initialization

    // Set up the stack pointers for each mode
    cpsid   i          // Disable interrupts

    // ABT mode
    ldr     r0, =__abt_sp
    msr     CPSR_c, #(ABT_MODE | I_Bit | F_Bit) // Switch to Abort mode
    mov     sp, r0

    // IRQ mode
    ldr     r0, =__irq_sp
    msr     CPSR_c, #(IRQ_MODE | I_Bit | F_Bit) // Switch to IRQ mode
    mov     sp, r0

    // SYS mode
    ldr     r0, =__sys_sp
    msr     CPSR_c, #(SYS_MODE | I_Bit | F_Bit) // Switch to System mode
    mov     sp, r0

    // UND mode
    ldr     r0, =__und_sp
    msr     CPSR_c, #(UND_MODE | I_Bit | F_Bit) // Switch to Undefined mode
    mov     sp, r0
    mov r2, #(0x01 << 2)

    // SVC mode
    ldr     r0, =__svc_sp
    msr     CPSR_c, #(SVC_MODE | I_Bit | F_Bit) // Switch to Supervisor mode
    mov     sp, r0
    // Leave processor in SVC mode

    // Store _vector_table at address 0x0000_0000
    ldr     r0, =_vector_table
    mov     r1, #0x0
    ldmia   r0!, {r2-r8}
    stmia   r1!, {r2-r8}

    // r1 = the last address+0x4

    // Store handler's addresses
    ldr     r0, =_vector_table_end
    ldmia   r0!, {r2-r8}
    stmia   r1!, {r2-r8}

    b main

/*
Check the current mode and stack pointer
 r0 = mode
 r1 = sp
*/
fn_check_mode_and_sp:
    mrs r0, cpsr
    and r0, r0, #0x1F
    mov r1, sp
