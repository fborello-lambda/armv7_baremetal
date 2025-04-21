.global main
.extern c_board_init
.extern c_putsln

.section .text.main
main: 
    // Init Board
    ldr r10, =c_board_init
    blx r10

    // Should never reach here
    ldr r0, =no_reach_msg
    ldr r10, =c_putsln
    blx r10

_idle:
    wfi
    b _idle

.section .data
no_reach_msg:
    .asciz "Should never reach here"
