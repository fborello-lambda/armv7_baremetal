.global _prefetch_handler

.extern c_prefetch_handler

.section .text._prefetch_handler

# Following:
# - https://developer.arm.com/documentation/den0013/d/Interrupt-Handling/External-interrupt-requests/Simplistic-interrupt-handling
# - https://developer.arm.com/documentation/den0013/d/Exception-Handling/Exception-priorities/The-return-instruction
_prefetch_handler:
    push {r0-r3, ip, lr}
    # bl clear_prefetch
    bl c_prefetch_handler
    pop {r0-r3, ip, lr}
    subs pc, lr, #4
