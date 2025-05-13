#include "inc/mmu.h"
#include "inc/gic.h"
#include "inc/timer.h"
#include "inc/uart.h"
#include <stdio.h>
#include <string.h>

// Clears memory
static void clear_memory(void *addr, uint32_t size_in_bytes) {
  uint32_t *p = (uint32_t *)addr;
  uint32_t count = size_in_bytes / 4; // Number of 32-bit words to clear

  for (uint32_t i = 0; i < count; i++) {
    p[i] = 0;
  }
}

extern uint32_t __stack_end[];

// REVISE
// It matches the length defined in the mmap.ld.
// Should be calculated instead of being hardcoded.
const uint32_t _STACK_SIZE = 20;

__attribute__((section(".text.mmu"))) void c_mmu_init(void) {
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

__attribute__((section(".text.mmu"))) void
c_mmu_fill_tables(mmu_tables_t *tables, uint32_t base_svc_stack_addr,
                  uint32_t base_irq_stack_addr) {
  // 1. Clean all Entries.
  clear_memory(tables->l1_table, L1_SIZE);
  for (int i = 0; i < L2_TABLES_PER_TASK; i++) {
    clear_memory(tables->l2_tables[i], L2_SIZE);
  }
  tables->next_l2_table = 0;

  // Map the MMU tables
  // Map 32 KB * 4 region starting at virtual 0x70080000 to physical 0x70080000
  identity_map_region(tables, 0x70080000, 0x70080000, 32 * 4);
  // Vector Table // Just some bytes are needed.
  c_mmu_map_4kb_page(tables, 0x00000000, 0x00000000, L2_DEFAULT_FLAGS);
  // Map the PUBLIC_RAM
  // TODO: Just the code of the task should be mapped.
  identity_map_region(tables, 0x70010000, 0x70010000, 16);
  // Map the Kernel's STACK
  identity_map_region(tables, (uint32_t)__stack_end, (uint32_t)__stack_end,
                      _STACK_SIZE);
  // Map the Task's SVC STACK
  c_mmu_map_4kb_page(tables, base_svc_stack_addr, base_svc_stack_addr,
                     L2_DEFAULT_FLAGS);
  // Map the Task's IRQ STACK
  c_mmu_map_4kb_page(tables, base_irq_stack_addr, base_irq_stack_addr,
                     L2_DEFAULT_FLAGS);
  // The following mapping is done on-demand by the c_abort_handler function.
  // c_mmu_map_4kb_page(tables, GICC0_ADDR, GICC0_ADDR, L2_DEFAULT_FLAGS);
  // c_mmu_map_4kb_page(tables, GICD0_ADDR, GICD0_ADDR, L2_DEFAULT_FLAGS);
  // c_mmu_map_4kb_page(tables, UART0_ADDR, UART0_ADDR, L2_DEFAULT_FLAGS);
  // c_mmu_map_4kb_page(tables, TIMER0_ADDR, TIMER0_ADDR, L2_DEFAULT_FLAGS);
}

__attribute__((section(".text.mmu"))) int32_t
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
  } else {
    l2_table = (uint32_t *)(tables->l1_table[l1_index] & 0xFFFFFC00);
  }

  uint32_t l2_index = (virt_addr >> 12) & 0xFF;
  if (l2_index >= L2_ENTRIES)
    return ERROR_L2_INDEX_OOR;
  if ((l2_table[l2_index] & 0xFFF) != 0)
    return ERROR_L2_IN_USE;

  l2_table[l2_index] = (phys_addr & 0xFFFFF000) | l2_flags;
  return PAGING_SUCCESS;
}

__attribute__((section(".text.mmu"))) int32_t
identity_map_region(mmu_tables_t *tables, uint32_t virt_addr,
                    uint32_t phys_addr, uint32_t size_in_kb) {
  uint32_t size_bytes = size_in_kb * 1024;
  uint32_t pages = (size_bytes + 0xFFF) / 0x1000;
  for (uint32_t i = 0; i < pages; i++) {
    int ret = c_mmu_map_4kb_page(tables, virt_addr + i * 0x1000,
                                 phys_addr + i * 0x1000, L2_DEFAULT_FLAGS);
    if (ret != PAGING_SUCCESS)
      return ret;
  }
  return PAGING_SUCCESS;
}
