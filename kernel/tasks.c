#include "inc/tasks.h"
#include "inc/sched.h"
#include "inc/uart.h"

__attribute__((section(".task0.text"))) void task_idle() {
  c_putsln("Task idle");
  while (1) {
    asm("wfi");
  }
}

__attribute__((section(".task1.text"))) void task1() {
  while (1) {
    static uint32_t local_var __attribute__((section(".task1.data"))) = 0;

    local_var++;
  }
}

__attribute__((section(".task2.text"))) void task2() {
  while (1) {
    static uint32_t local_var __attribute__((section(".task2.data"))) =
        0xFFFFFFFF;

    local_var--;
  }
}
