#include "../sys/inc/logger.h"
#include "inc/mmu.h"

__attribute__((section(".text._abort_handler"))) uint32_t c_abort_handler() {
  // Get the fault address from DFAR
  // # Following:
  // -
  // https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Control-Registers-in-a-PMSA-implementation/PMSA-System-control-registers-descriptions--in-register-order/DFAR--Data-Fault-Address-Register--PMSA
  uint32_t fault_addr;
  __asm__ volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(fault_addr));
  c_log_warn("Abort Handler at address");
  c_puts_hex(fault_addr);
  c_putchar('\n');

  while (1) {
    asm("wfi");
  }
  // TODO: The following piece of code could be used to fill pages on demand.
  // It has to be revised and the flags should be set as needed.
  // A better algorithm is needed.
  // Get the TTBR0
  // # Following:
  // https://developer.arm.com/documentation/ddi0406/cb/System-Level-Architecture/System-Control-Registers-in-a-VMSA-implementation/VMSA-System-control-registers-descriptions--in-register-order/TTBR0--Translation-Table-Base-Register-0--VMSA?lang=en
  uint32_t *ttbr0;
  __asm__ volatile("mrc p15, 0, %0, c2, c0, 0" : "=r"(ttbr0));

  // The ttbr0 is the base address of the L1 table
  // which is the base address of the mmu_tables_t structure.
  mmu_tables_t *tables = (mmu_tables_t *)ttbr0;
  return c_mmu_map_4kb_page(tables, fault_addr, fault_addr, L2_DEFAULT_FLAGS);
}
