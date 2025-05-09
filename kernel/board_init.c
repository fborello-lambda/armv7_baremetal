#include "inc/gic.h"
#include "inc/mmu.h"
#include "inc/sched.h"
#include "inc/tasks.h"
#include "inc/timer.h"
#include "inc/uart.h"

extern uint32_t g_kernel_l1_table[];

extern __attribute__((section(".text"))) void c_board_init(void) {
  c_mmu_init(g_kernel_l1_table);
  c_gic_init();
  c_timer_init();

  // Seems that the initialization is not necessary
  // on Realview pb8
  // Init UART
  c_UART0_init();

  // Init Tasks / Scheduler
  c_scheduler_init();

  c_putsln("Should not reach here");
  return;
}
