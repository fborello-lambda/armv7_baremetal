#include "inc/gic.h"
#include "inc/mmu.h"
#include "inc/sched.h"
#include "inc/tasks.h"
#include "inc/timer.h"
#include "inc/uart.h"

static void copy_lma_into_phy(void *phy, const void *lma, uint32_t size) {
  if (size == 0) {
    return;
  }
  uint8_t *d = (uint8_t *)phy;
  const uint8_t *s = (const uint8_t *)lma;
  while (size--) {
    *d++ = *s++;
  }
}

static void copy_sections(void) {
  copy_lma_into_phy(&_KERNEL_TEXT_PHY, &_KERNEL_TEXT_LMA,
                    GET_SYMBOL_VALUE(_KERNEL_TEXT_SIZE));
  copy_lma_into_phy(&_KERNEL_DATA_PHY, &_KERNEL_DATA_LMA,
                    GET_SYMBOL_VALUE(_KERNEL_DATA_SIZE));
  copy_lma_into_phy(&_KERNEL_RODATA_PHY, &_KERNEL_RODATA_LMA,
                    GET_SYMBOL_VALUE(_KERNEL_RODATA_SIZE));

  copy_lma_into_phy(&_TASK0_TEXT_PHY, &_TASK0_TEXT_LMA,
                    GET_SYMBOL_VALUE(_TASK0_TEXT_SIZE));

  copy_lma_into_phy(&_TASK1_TEXT_PHY, &_TASK1_TEXT_LMA,
                    GET_SYMBOL_VALUE(_TASK1_TEXT_SIZE));
  copy_lma_into_phy(&_TASK1_DATA_PHY, &_TASK1_DATA_LMA,
                    GET_SYMBOL_VALUE(_TASK1_DATA_SIZE));
  copy_lma_into_phy(&_TASK1_RODATA_PHY, &_TASK1_RODATA_LMA,
                    GET_SYMBOL_VALUE(_TASK1_RODATA_SIZE));

  copy_lma_into_phy(&_TASK2_TEXT_PHY, &_TASK2_TEXT_LMA,
                    GET_SYMBOL_VALUE(_TASK2_TEXT_SIZE));
  copy_lma_into_phy(&_TASK2_DATA_PHY, &_TASK2_DATA_LMA,
                    GET_SYMBOL_VALUE(_TASK2_DATA_SIZE));
  copy_lma_into_phy(&_TASK2_RODATA_PHY, &_TASK2_RODATA_LMA,
                    GET_SYMBOL_VALUE(_TASK2_RODATA_SIZE));
}

__attribute__((section(".text"))) void c_board_init(void) {
  copy_sections();
  c_gic_init();
  c_timer_init();

  // Seems that the initialization is not necessary
  // on Realview pb8
  // Init UART
  c_UART0_init();
  asm volatile("cpsie i");
  c_putsln("Starting Scheduler...");
  asm volatile("cpsid i");
  asm volatile("isb");

  // Init Tasks / Scheduler
  // It also fills the tables needed to start the MMU
  c_scheduler_init();

  c_putsln("Should not reach here");
  return;
}
