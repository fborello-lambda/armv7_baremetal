.global _abort_handler
.section .text._abort_handler

# Following:
# - https://developer.arm.com/documentation/den0013/d/Interrupt-Handling/External-interrupt-requests/Simplistic-interrupt-handling
# - https://developer.arm.com/documentation/den0013/d/Exception-Handling/Exception-priorities/The-return-instruction
_abort_handler:
    push {r0-r3, ip, lr}
    bl c_abort_handler
    pop {r0-r3, ip, lr}
    subs pc, lr, #8
