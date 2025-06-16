.global _swi_handler

.extern c_puts
.extern c_putchar
.extern c_puts_hex

# Following:
# - https://developer.arm.com/documentation/den0013/d/Interrupt-Handling/External-interrupt-requests/Simplistic-interrupt-handling
# - https://developer.arm.com/documentation/den0013/d/Exception-Handling/Exception-priorities/The-return-instruction
.section .text._swi_handler
_swi_handler:
    push {r0-r5, ip, lr}
    # bl clear_swi
    # bl c_swi_handler
    mov r5, lr // <--- VERY IMPORTANT
    // The lr is later used to link back to this handler after a function call

    ldr r0, =msg
    ldr r10, =c_puts
    blx r10

    # Get the svc <imm> number
    sub r5, r5, #4          // Skip the current instruction
    ldr r0, [r5]            // Read the instruction
    // Mask out the instr to get the imm value 0 to (2^24)-1 (a 24-bit value) in an ARM instruction
    ldr r2, =#0xFFFFFF
    and r0, r0, r2

    ldr r10, =c_puts_hex
    blx r10

    mov r0, #'\n'
    ldr r10, =c_putchar
    blx r10

    pop {r0-r5, ip, lr}
    movs pc, lr

.section .data
msg:
    .asciz "SWI "
