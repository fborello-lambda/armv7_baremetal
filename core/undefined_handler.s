.global _undefined_handler

.extern c_undefined_handler

.section .text._undefined_handler
_undefined_handler:
    push {r0-r5, ip, lr}
    bl c_undefined_handler
    pop {r0-r5, ip, lr}
    movs pc, lr
