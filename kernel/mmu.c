#include "inc/mmu.h"
#include "inc/gic.h"
#include "inc/timer.h"
#include "inc/uart.h"
#include <stdio.h>
#include <string.h>

uint32_t g_kernel_l1_table[L1_ENTRIES]
    __attribute__((section(".l1_table"), aligned(L1_ALIGN))) = {0};
uint32_t *g_next_l2_addr = g_kernel_l1_table + L1_SIZE;

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
const uint32_t _STACK_SIZE = 13;

__attribute__((section(".text"))) void
c_mmu_fill_tables(uint32_t *base_l1_ptr, uint32_t base_svc_stack_addr,
                  uint32_t base_irq_stack_addr) {
  // 1. Clean all Entries.
  clear_memory(base_l1_ptr, L1_SIZE);

  // Map the MMU tables
  // Map 20 KB * 4 region starting at virtual 0x70080000 to physical 0x70080000
  identity_map_region(base_l1_ptr, 0x70080000, 0x70080000, 20 * 4);

  // Vector Table // Just some bytes are needed.
  c_mmu_map_4kb_page(base_l1_ptr, 0x00000000, 0x00000000, L2_DEFAULT_FLAGS);

  // TODO, it should map only the code for the task and the code for the kernel.
  // Map the PUBLIC_RAM
  // Map 16 KB region starting at virtual 0x70010000 to physical 0x70010000
  identity_map_region(base_l1_ptr, 0x70010000, 0x70010000, 16);

  // Map the STACK
  // Map 21 KB region starting at virtual 0x70020000 to physical 0x70020000
  identity_map_region(base_l1_ptr, 0x70020000, 0x70020000, 21);
  // TODO, it should only map the task's stack.
  /*
  // Map the Kernel STACK
  identity_map_region(base_l1_ptr, (uint32_t)__stack_end, (uint32_t)__stack_end,
  _STACK_SIZE);

  // Map the SVC STACK
  c_mmu_map_4kb_page(base_l1_ptr, base_svc_stack_addr, base_svc_stack_addr,
  L2_DEFAULT_FLAGS);

  // Map the IRQ STACK
  c_mmu_map_4kb_page(base_l1_ptr, base_irq_stack_addr, base_irq_stack_addr,
  L2_DEFAULT_FLAGS);
    */

  // Map UART0_ADRR, GICC0_ADDR, GICD0_ADDR, TIMER0_ADDR on demand.
  // The _abort_handler calls the c_abort_handler and maps the
  // address that couldn't be accessed.
  c_mmu_map_4kb_page(base_l1_ptr, GICC0_ADDR, GICC0_ADDR, L2_DEFAULT_FLAGS);
  c_mmu_map_4kb_page(base_l1_ptr, GICD0_ADDR, GICD0_ADDR, L2_DEFAULT_FLAGS);
  c_mmu_map_4kb_page(base_l1_ptr, UART0_ADDR, UART0_ADDR, L2_DEFAULT_FLAGS);
  c_mmu_map_4kb_page(base_l1_ptr, TIMER0_ADDR, TIMER0_ADDR, L2_DEFAULT_FLAGS);
}

__attribute__((section(".text"))) void c_mmu_init(void) {
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

// Maps a 4KB page
// Requires a Virtual and Physical address.
// The Virtual address is mapped to the Physical address.
int32_t c_mmu_map_4kb_page(uint32_t *base_l1_ptr, uint32_t virt_addr,
                           uint32_t phys_addr, uint32_t l2_flags) {
  // Obtain the index within the L1 page.
  // For example, if it is 0x7012_0000 >> 20
  // The l1_index is 0x701
  uint32_t l1_index = virt_addr >> 20;
  if (l1_index >= L1_ENTRIES) {
    return ERROR_L1_INDEX_OOR;
  }

  uint32_t *l2_table;

  // Allocate an L2 table if not present
  // We are checking if the entry at the l1_index
  // obtained above has something.
  // Basically checking the first 2 bits.
  if ((base_l1_ptr[l1_index] & 0x3) == 0) {
    // Allocate a new L2 table at next_l2_addr
    l2_table = g_next_l2_addr;
    clear_memory(l2_table, L2_SIZE);

    // Update L1 entry to point to new L2 table with coarse table flag
    base_l1_ptr[l1_index] =
        ((uintptr_t)l2_table & 0xFFFFFC00) | L1_TYPE_COARSE_TABLE;

    // Advance next_l2_addr for next allocation
    g_next_l2_addr += L2_SIZE;
  } else {
    // L2 table already exists, get its address from L1 entry
    l2_table = (uint32_t *)(base_l1_ptr[l1_index] & 0xFFFFFC00);
  }

  // Calculate L2 index (bits [19:12])
  // For example, if it is 0x7012_0000 >> 12
  // The l2_index is 20
  uint32_t l2_index = (virt_addr >> 12) & 0xFF;
  if (l2_index >= L2_ENTRIES) {
    return ERROR_L2_INDEX_OOR;
  }

  // Check if entry is already set (flags != 0 means entry is in use)
  if ((l2_table[l2_index] & 0xFFF) != 0) {
    return ERROR_L2_IN_USE;
  }

  // Set L2 entry to map phys_addr with flags
  // Bits [31:12] -> BaseAddr
  // The rest are flags.
  l2_table[l2_index] = (phys_addr & 0xFFFFF000) | l2_flags;

  return PAGING_SUCCESS;
}

// Maps a region of size_in_kb (rounded up to 4KB pages)
int32_t identity_map_region(uint32_t *base_l1_ptr, uint32_t virt_addr,
                            uint32_t phys_addr, uint32_t size_in_kb) {
  uint32_t size_bytes = size_in_kb * 1024;
  // Calculate number of 4KB pages (round up)
  uint32_t pages = (size_bytes + 0xFFF) / 0x1000;

  for (uint32_t i = 0; i < pages; i++) {
    int ret = c_mmu_map_4kb_page(base_l1_ptr, virt_addr + i * 0x1000,
                                 phys_addr + i * 0x1000, L2_DEFAULT_FLAGS);
    if (ret != PAGING_SUCCESS) {
      return ret;
    }
  }
  return PAGING_SUCCESS;
}
