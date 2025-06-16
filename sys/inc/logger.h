#ifndef __LOGGER_LIB_H
#define __LOGGER_LIB_H

#include "../../kernel/inc/uart.h"
#include <stdint.h>

void c_log_error(const char *s);
void c_log_info(const char *s);
void c_log_warn(const char *s);
void c_log_mapping(const char *label, uint32_t vaddr, uint32_t paddr,
                   uint32_t size);
void c_log_page(uint32_t vaddr, uint32_t paddr);
void c_log_taskswitch(uint8_t task_id);

#endif // __LOGGER_LIB_H
