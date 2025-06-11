#include "inc/mmu.h"
#include "inc/gic.h"
#include "inc/timer.h"
#include "inc/uart.h"
#include <stdio.h>
#include <string.h>

// Clears memory
void clear_memory(void *addr, uint32_t size_in_bytes) {
  uint32_t *p = (uint32_t *)addr;
  uint32_t count = size_in_bytes / 4; // Number of 32-bit words to clear

  for (uint32_t i = 0; i < count; i++) {
    p[i] = 0;
  }
}

__attribute__((section(".kernel.text.mmu"))) void
log_mapping(const char *label, uint32_t vaddr, uint32_t paddr, uint32_t size) {
  c_puts("\033[1;32m");
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

__attribute__((section(".kernel.text.mmu"))) void log_page(uint32_t vaddr,
                                                           uint32_t paddr) {
  // Logging style matching your log_mapping()
  c_puts("  \033[1;34mMapping Page:\033[0m VA = ");
  c_puts_hex(vaddr);
  c_puts(" -> PA = ");
  c_puts_hex(paddr);
  c_putsln("");
}

__attribute__((section(".kernel.text.mmu"))) void c_mmu_init(void) {
  // Set DACR to manager (all access)
  __asm__ volatile("ldr r0, =0xFFFFFFFF\n"
                   "mcr p15, 0, r0, c3, c0, 0\n" ::
                       : "r0");
  // Enable MMU (set M bit in SCTLR)
  __asm__ volatile("mrc p15, 0, r0, c1, c0, 0\n"
                   "orr r0, r0, #0x1\n"
                   "mcr p15, 0, r0, c1, c0, 0\n"
                   "isb\n" ::
                       : "r0");
}

__attribute__((section(".kernel.text.mmu"))) void
c_mmu_fill_tables(mmu_tables_t *tables, uint32_t task_id) {
  clear_memory(tables->l1_table, L1_SIZE);
  for (int i = 0; i < L2_TABLES_PER_TASK; i++) {
    clear_memory(tables->l2_tables[i], L2_SIZE);
  }
  tables->next_l2_table = 0;

  log_mapping("Mapping RGN", 0x70010000, 0x70010000, 4 * 1024);
  map_region(tables, 0x70010000, 0x70010000, 4 * 1024);

  log_mapping(".kernel.text", (uint32_t)&_KERNEL_TEXT_VMA,
              (uint32_t)&_KERNEL_TEXT_PHY, GET_SYMBOL_VALUE(_KERNEL_TEXT_SIZE));
  map_region(tables, (uint32_t)&_KERNEL_TEXT_VMA, (uint32_t)&_KERNEL_TEXT_PHY,
             GET_SYMBOL_VALUE(_KERNEL_TEXT_SIZE));

  log_mapping(".kernel.data", (uint32_t)&_KERNEL_DATA_VMA,
              (uint32_t)&_KERNEL_DATA_PHY, GET_SYMBOL_VALUE(_KERNEL_DATA_SIZE));
  map_region(tables, (uint32_t)&_KERNEL_DATA_VMA, (uint32_t)&_KERNEL_DATA_PHY,
             GET_SYMBOL_VALUE(_KERNEL_DATA_SIZE));

  log_mapping(".kernel.rodata", (uint32_t)&_KERNEL_RODATA_VMA,
              (uint32_t)&_KERNEL_RODATA_PHY,
              GET_SYMBOL_VALUE(_KERNEL_RODATA_SIZE));
  map_region(tables, (uint32_t)&_KERNEL_RODATA_VMA,
             (uint32_t)&_KERNEL_RODATA_PHY,
             GET_SYMBOL_VALUE(_KERNEL_RODATA_SIZE));

  log_mapping(".kernel.bss", (uint32_t)&_KERNEL_BSS_VMA,
              (uint32_t)&_KERNEL_BSS_PHY, GET_SYMBOL_VALUE(_KERNEL_BSS_SIZE));
  map_region(tables, (uint32_t)&_KERNEL_BSS_VMA, (uint32_t)&_KERNEL_BSS_PHY,
             GET_SYMBOL_VALUE(_KERNEL_BSS_SIZE));

  log_mapping(".kernel.stack", (uint32_t)&_KERNEL_STACK,
              (uint32_t)&_KERNEL_STACK, GET_SYMBOL_VALUE(_KERNEL_STACK_SIZE));
  map_region(tables, (uint32_t)&_KERNEL_STACK, (uint32_t)&_KERNEL_STACK,
             GET_SYMBOL_VALUE(_KERNEL_STACK_SIZE));

  // MMU
  log_mapping("MMU region", 0x70080000, 0x70080000, 32 * 3 * 1024);
  map_region(tables, 0x70080000, 0x70080000, 32 * 3 * 1024);

  // Vector Table
  log_mapping("Vector Table", 0x00000000, 0x00000000, 4 * 1024);
  c_mmu_map_4kb_page(tables, 0x00000000, 0x00000000, L2_DEFAULT_FLAGS);

  // Peripherals
  log_mapping("GICC0", GICC0_ADDR, GICC0_ADDR, 4 * 1024);
  c_mmu_map_4kb_page(tables, GICC0_ADDR, GICC0_ADDR, L2_DEFAULT_FLAGS);

  log_mapping("GICD0", GICD0_ADDR, GICD0_ADDR, 4 * 1024);
  c_mmu_map_4kb_page(tables, GICD0_ADDR, GICD0_ADDR, L2_DEFAULT_FLAGS);

  log_mapping("UART0", UART0_ADDR, UART0_ADDR, 4 * 1024);
  c_mmu_map_4kb_page(tables, UART0_ADDR, UART0_ADDR, L2_DEFAULT_FLAGS);

  log_mapping("TIMER0", TIMER0_ADDR, TIMER0_ADDR, 4 * 1024);
  c_mmu_map_4kb_page(tables, TIMER0_ADDR, TIMER0_ADDR, L2_DEFAULT_FLAGS);

  switch (task_id) {
  case 0:
    log_mapping("TASK0 .text", (uint32_t)&_TASK0_TEXT_VMA,
                (uint32_t)&_TASK0_TEXT_PHY, GET_SYMBOL_VALUE(_TASK0_TEXT_SIZE));
    map_region(tables, (uint32_t)&_TASK0_TEXT_VMA, (uint32_t)&_TASK0_TEXT_PHY,
               GET_SYMBOL_VALUE(_TASK0_TEXT_SIZE));
    break;

  case 1:
    log_mapping("TASK1 .text", (uint32_t)&_TASK1_TEXT_VMA,
                (uint32_t)&_TASK1_TEXT_PHY, GET_SYMBOL_VALUE(_TASK1_TEXT_SIZE));
    map_region(tables, (uint32_t)&_TASK1_TEXT_VMA, (uint32_t)&_TASK1_TEXT_PHY,
               GET_SYMBOL_VALUE(_TASK1_TEXT_SIZE));

    log_mapping("TASK1 .data", (uint32_t)&_TASK1_DATA_VMA,
                (uint32_t)&_TASK1_DATA_PHY, GET_SYMBOL_VALUE(_TASK1_DATA_SIZE));
    map_region(tables, (uint32_t)&_TASK1_DATA_VMA, (uint32_t)&_TASK1_DATA_PHY,
               GET_SYMBOL_VALUE(_TASK1_DATA_SIZE));

    log_mapping("TASK1 .rodata", (uint32_t)&_TASK1_RODATA_VMA,
                (uint32_t)&_TASK1_RODATA_PHY,
                GET_SYMBOL_VALUE(_TASK1_RODATA_SIZE));
    map_region(tables, (uint32_t)&_TASK1_RODATA_VMA,
               (uint32_t)&_TASK1_RODATA_PHY,
               GET_SYMBOL_VALUE(_TASK1_RODATA_SIZE));

    log_mapping("TASK1 .bss", (uint32_t)&_TASK1_BSS_VMA,
                (uint32_t)&_TASK1_BSS_PHY, GET_SYMBOL_VALUE(_TASK1_BSS_SIZE));
    map_region(tables, (uint32_t)&_TASK1_BSS_VMA, (uint32_t)&_TASK1_BSS_PHY,
               GET_SYMBOL_VALUE(_TASK1_BSS_SIZE));

    log_mapping("TASK1 .stack", (uint32_t)&_TASK1_STACK,
                (uint32_t)&_TASK1_STACK_PHY,
                GET_SYMBOL_VALUE(_TASK1_STACK_SIZE));
    map_region(tables, (uint32_t)&_TASK1_STACK, (uint32_t)&_TASK1_STACK_PHY,
               GET_SYMBOL_VALUE(_TASK1_STACK_SIZE));

    // log_mapping("TASK1 RAREA", (uint32_t)&_TASK1_RAREA_START_VMA,
    //             (uint32_t)&_TASK1_RAREA_START_PHY, TASK1_RAREA_SIZE_B);
    // map_region(tables, (uint32_t)&_TASK1_RAREA_START_VMA,
    //            (uint32_t)&_TASK1_RAREA_START_PHY, TASK1_RAREA_SIZE_B);
    break;

  case 2:
    log_mapping("TASK2 .text", (uint32_t)&_TASK2_TEXT_VMA,
                (uint32_t)&_TASK2_TEXT_PHY, GET_SYMBOL_VALUE(_TASK2_TEXT_SIZE));
    map_region(tables, (uint32_t)&_TASK2_TEXT_VMA, (uint32_t)&_TASK2_TEXT_PHY,
               GET_SYMBOL_VALUE(_TASK2_TEXT_SIZE));

    log_mapping("TASK2 .data", (uint32_t)&_TASK2_DATA_VMA,
                (uint32_t)&_TASK2_DATA_PHY, GET_SYMBOL_VALUE(_TASK2_DATA_SIZE));
    map_region(tables, (uint32_t)&_TASK2_DATA_VMA, (uint32_t)&_TASK2_DATA_PHY,
               GET_SYMBOL_VALUE(_TASK2_DATA_SIZE));

    log_mapping("TASK2 .rodata", (uint32_t)&_TASK2_RODATA_VMA,
                (uint32_t)&_TASK2_RODATA_PHY,
                GET_SYMBOL_VALUE(_TASK2_RODATA_SIZE));
    map_region(tables, (uint32_t)&_TASK2_RODATA_VMA,
               (uint32_t)&_TASK2_RODATA_PHY,
               GET_SYMBOL_VALUE(_TASK2_RODATA_SIZE));

    log_mapping("TASK2 .bss", (uint32_t)&_TASK2_BSS_VMA,
                (uint32_t)&_TASK2_BSS_PHY, GET_SYMBOL_VALUE(_TASK2_BSS_SIZE));
    map_region(tables, (uint32_t)&_TASK2_BSS_VMA, (uint32_t)&_TASK2_BSS_PHY,
               GET_SYMBOL_VALUE(_TASK2_BSS_SIZE));

    log_mapping("TASK2 .stack", (uint32_t)&_TASK2_STACK,
                (uint32_t)&_TASK2_STACK_PHY,
                GET_SYMBOL_VALUE(_TASK2_STACK_SIZE));
    map_region(tables, (uint32_t)&_TASK2_STACK, (uint32_t)&_TASK2_STACK_PHY,
               GET_SYMBOL_VALUE(_TASK2_STACK_SIZE));

    // log_mapping("TASK2 RAREA", (uint32_t)&_TASK2_RAREA_START_VMA,
    //             (uint32_t)&_TASK2_RAREA_START_PHY, TASK2_RAREA_SIZE_B);
    // map_region(tables, (uint32_t)&_TASK2_RAREA_START_VMA,
    //            (uint32_t)&_TASK2_RAREA_START_PHY, TASK2_RAREA_SIZE_B);
    break;

  default:
    break;
  }

  c_putsln("\033[1;31m[INFO] Pagination Done \033[0m");
}

__attribute__((section(".kernel.text.mmu"))) int32_t
c_mmu_map_4kb_page(mmu_tables_t *tables, uint32_t virt_addr, uint32_t phys_addr,
                   uint32_t l2_flags) {
  uint32_t l1_index = virt_addr >> 20;
  if (l1_index >= L1_ENTRIES)
    return ERROR_L1_INDEX_OOR;

  uint32_t *l2_table;
  // Allocate an L2 table if not present
  if ((tables->l1_table[l1_index] & 0x3) == 0) {
    if (tables->next_l2_table >= L2_TABLES_PER_TASK)
      return ERROR_L2_INDEX_OOR;
    l2_table = tables->l2_tables[tables->next_l2_table++];
    clear_memory(l2_table, L2_SIZE);
    tables->l1_table[l1_index] =
        ((uintptr_t)l2_table & 0xFFFFFC00) | L1_TYPE_COARSE_TABLE;
    c_puts("[MMU] New L2 table for VA range: ");
    c_puts_hex(l1_index << 20);
    c_puts(" - ");
    c_puts_hex((l1_index << 20) + 0xFFFFF); // end of 1MB range
    c_puts(" -> L2 table at PA: ");
    c_puts_hex((uint32_t)l2_table);
    c_putsln("");
  } else {
    l2_table = (uint32_t *)(tables->l1_table[l1_index] & 0xFFFFFC00);
  }

  uint32_t l2_index = (virt_addr >> 12) & 0xFF;
  if (l2_index >= L2_ENTRIES)
    return ERROR_L2_INDEX_OOR;
  if (l2_table[l2_index] != 0)
    return ERROR_L2_IN_USE;

  l2_table[l2_index] = (phys_addr & 0xFFFFF000) | l2_flags;
  log_page(virt_addr, phys_addr);
  return PAGING_SUCCESS;
}

__attribute__((section(".kernel.text.mmu"))) int32_t
map_region(mmu_tables_t *tables, uint32_t virt_addr, uint32_t phys_addr,
           uint32_t size_in_bytes) {
  uint32_t pages = (size_in_bytes + 0xFFF) / 0x1000;
  for (uint32_t i = 0; i < pages; i++) {
    uint32_t va = virt_addr + i * 0x1000;
    uint32_t pa = phys_addr + i * 0x1000;

    int ret = c_mmu_map_4kb_page(tables, va, pa, L2_DEFAULT_FLAGS);
    if (ret != PAGING_SUCCESS) {
      c_puts("\033[1;31m[ERROR] Failed to map region: \033[0m");
      c_puts_hex(virt_addr);
      c_putsln("");
      return ret;
    }
  }
  return PAGING_SUCCESS;
}
