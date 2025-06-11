#include "../sys/inc/logger.h"

__attribute__((section(".text._prefetch_handler"))) uint32_t
c_prefetch_handler() {
  uint32_t fault_addr;
  __asm__ volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(fault_addr));
  c_log_error("Prefetch Handler");
  c_puts_hex(fault_addr);
  while (1) {
    asm("wfi");
  }
}
