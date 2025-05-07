#include "inc/mmu.h"
__attribute__((section(".text._abort_handler"))) uint32_t c_abort_handler() {
  // Get the fault address from the fault status register
  // # Following:
  // -
  // https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Control-Registers-in-a-PMSA-implementation/PMSA-System-control-registers-descriptions--in-register-order/DFAR--Data-Fault-Address-Register--PMSA
  uint32_t fault_addr;
  __asm__ volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(fault_addr));

  return c_mmu_map_4kb_page(fault_addr, fault_addr, L2_DEFAULT_FLAGS);
}
