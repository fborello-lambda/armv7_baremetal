#include "inc/gic.h"
#include "inc/sched.h"
#include "inc/timer.h"
#include "inc/uart.h"
// CTX should have a struct that reflects the pushed data inside the
// asm_irq_handler
__attribute__((section(".text._irq_handler"))) uint32_t
c_irq_handler(_ctx_t *ctx) {
  _gicc_t *const GICC0 = (_gicc_t *)GICC0_ADDR;
  _timer_t *const TIMER0 = (_timer_t *)TIMER0_ADDR;

  // Interrupt acknowledge register
  // It tells whcih interrupt id has been triggered
  uint32_t id = GICC0->IAR;

  uint32_t ret_sp = (uint32_t)ctx->sp;

  switch (id) {
  case GIC_SOURCE_TIMER0:
    TIMER0->Timer1IntClr = 0x1;
    c_systick_handler();
    ret_sp = c_scheduler(ctx);
    break;

  default:
    break;
  }

  // End of Interrupt
  // When the interrupt has been completed by the
  // processor, it writes the interrupt number to this
  // register in the interrupting GIC
  GICC0->EOIR = id;

  return ret_sp;
}
