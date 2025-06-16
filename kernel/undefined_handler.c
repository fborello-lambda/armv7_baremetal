#include "../sys/inc/logger.h"

__attribute__((section(".text._undefined_handler"))) uint32_t
c_undefined_handler() {
  c_log_error("Undefined Handler Exception");
  c_putchar('\n');

  while (1) {
    asm("wfi");
  }
}
