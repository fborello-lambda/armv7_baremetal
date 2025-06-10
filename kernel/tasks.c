#include "inc/tasks.h"
#include "inc/mmu.h"
#include "inc/sched.h"
#include "inc/uart.h"

__attribute__((section(".task0.text"))) void task_idle() {
  c_putsln("Task idle");
  while (1) {
    asm("wfi");
  }
}

__attribute__((section(".task1.rodata"))) static const char str_task1[] =
    "[TASK1]";
__attribute__((section(".task1.text"))) void task1() {
  volatile uint32_t *addr = (uint32_t *)&_TASK1_RAREA_START_VMA;
  uint32_t size_in_bytes = TASK2_RAREA_SIZE_KB * 1024;
  uint32_t num_words = size_in_bytes / sizeof(uint32_t);
  uint32_t temp_original;

  c_putsln(str_task1);
  while (1) {
    c_putsln(str_task1);
    for (uint32_t i = 0; i < num_words; i++) {
      // Save original
      temp_original = addr[i];

      // Write new value
      addr[i] = 0x55AA55AA;

      // Verify
      if (addr[i] != 0x55AA55AA) {
        // Here there should be an error
      }

      // Restore
      addr[i] = temp_original;
    }
  }
}

__attribute__((section(".task2.rodata"))) static const char str_task2[] =
    "[TASK2]";
__attribute__((section(".task2.text"))) void task2() {
  volatile uint32_t *addr = (uint32_t *)&_TASK2_RAREA_START_VMA;
  uint32_t size_in_bytes = TASK2_RAREA_SIZE_KB * 1024;
  uint32_t num_words = size_in_bytes / sizeof(uint32_t);
  c_putsln(str_task2);
  while (1) {
    c_putsln(str_task2);
    for (uint32_t i = 0; i < num_words; i++) {

      uint32_t val = addr[i];

      addr[i] = ~val;
    }
  }
}
