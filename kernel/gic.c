#include "inc/gic.h"

__attribute__((section(".kernel.text"))) void c_gic_init() {
  _gicc_t *const GICC0 = (_gicc_t *)GICC0_ADDR;
  _gicd_t *const GICD0 = (_gicd_t *)GICD0_ADDR;

  // Priority Mask
  // A Priority mask value of 0xF means interrupts with
  // priority 0xF are masked but interrupts with higher
  // priority values 0x0 to 0xE are not masked
  GICC0->PMR = 0x000000F0;
  // Enable interrupt 36 (Timer)
  GICD0->ISENABLER[1] |= 0x00000010;
  // Enable interrupt 44 (UART0)
  GICD0->ISENABLER[1] |= 0x00001000;
  // Enable the CPU interface for this GIC
  GICC0->CTLR = 0x00000001;
  // Enable the CPU interface for this GIC
  GICD0->CTLR = 0x00000001;
}
