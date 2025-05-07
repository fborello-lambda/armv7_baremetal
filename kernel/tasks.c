#include "inc/tasks.h"
#include "inc/sched.h"
#include "inc/uart.h"

static uint32_t global0_var = 0;
static uint32_t global1_var = 0xFFFFFFFF;

__attribute__((section(".text.task_idle"))) void task_idle() {
  c_putsln("Task idle");
  while (1) {
    asm("wfi");
  }
}

__attribute__((section(".text.task_1"))) void task1() {
  while (1) {
    static uint32_t local_var = 0;

    local_var++;
    global0_var++;
  }
}

__attribute__((section(".text.task_2"))) void task2() {
  while (1) {
    static uint32_t local_var = 0xFFFFFFFF;

    local_var--;
    global1_var--;
  }
}

__attribute__((section(".text.task_3"))) void task3() {
  while (1) {
    global1_var++;
    global0_var--;
  }
}
