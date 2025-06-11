#include "inc/logger.h"

__attribute__((section(".text"))) void c_log_error(const char *s) {
  c_puts("\033[1;31m[ERROR] \033[0m");
  c_putsln(s);
}
__attribute__((section(".text"))) void c_log_info(const char *s) {
  c_puts("\033[1;32m[INFO] \033[0m");
  c_putsln(s);
}
__attribute__((section(".text"))) void c_log_warn(const char *s) {
  c_puts("\033[1;33m[WARN] \033[0m");
  c_putsln(s);
}

__attribute__((section(".text"))) void c_log_mapping(const char *label,
                                                     uint32_t vaddr,
                                                     uint32_t paddr,
                                                     uint32_t size) {
  c_puts("\033[1;36m");
  c_puts(label);
  c_putsln(":\033[0m");
  c_puts("VA = ");
  c_puts_hex(vaddr);
  c_puts(" -> PA = ");
  c_puts_hex(paddr);
  c_puts(" (size = ");
  c_puts_hex(size);
  c_putsln(" bytes)");
}

__attribute__((section(".text"))) void c_log_page(uint32_t vaddr,
                                                  uint32_t paddr) {
  c_puts("  \033[1;34mMapping Page:\033[0m VA = ");
  c_puts_hex(vaddr);
  c_puts(" -> PA = ");
  c_puts_hex(paddr);
  c_putsln("");
}
