#include "inc/timer.h"

__attribute__((section(".kernel.text"))) void c_timer_init() {
  _timer_t *const TIMER0 = (_timer_t *)TIMER0_ADDR;

  // The TimerXLoad Register is a 32-bit register that contains the value from
  // which the counter is to decrement. This is the value used to reload the
  // counter when Periodic mode is enabled, and the current count reaches zero.
  // Load with 65536 (decimal)
  TIMER0->Timer1Load = 0x00010000;
  // Set to 32-bit counter
  TIMER0->Timer1Ctrl = 0x00000002;
  // Timer in periodic mode
  TIMER0->Timer1Ctrl |= 0x00000040;
  // Timer interrupt enable
  TIMER0->Timer1Ctrl |= 0x00000020;
  // Timer Enabled
  TIMER0->Timer1Ctrl |= CTRL_IRQ_ENABLE;
}
