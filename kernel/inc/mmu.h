#ifndef __MMU_LIB_H__
#define __MMU_LIB_H__

#include <stdint.h>

#define L1_ENTRIES 0x1000 // 4096 entries
#define L1_SIZE 0x4000    // 16KB (4096 * 4 bytes)
#define L1_ALIGN 0x4000   // 16KB alignment

#define L2_ENTRIES 0x100 // 256 entries
#define L2_SIZE 0x0400   // 1KB (256 * 4 bytes)

// Matches the length defined in the mmap.ld.
// Should be calculated instead of being hardcoded.
#define L2_TABLES_PER_TASK 8 // 8 L2 tables per task

typedef struct {
  uint32_t l1_table[L1_ENTRIES] __attribute__((aligned(L1_SIZE)));
  uint32_t l2_tables[L2_TABLES_PER_TASK][L2_ENTRIES]
      __attribute__((aligned(L2_SIZE)));
  uint32_t next_l2_table; // This variable is used to keep track of the next L2
                          // table to be used
  // It causes the L2 tables to be allocated in a round-robin fashion.
} mmu_tables_t;

// Paging Descriptor flags
#define L1_TYPE_COARSE_TABLE 0x1
// L2 flags (small page, AP=RW, cacheable, bufferable, executable)
#define L2_DEFAULT_FLAGS 0x32

#define ERROR_L1_INDEX_OOR -1
#define ERROR_L2_INDEX_OOR -2
#define ERROR_L2_IN_USE -3
#define PAGING_SUCCESS 0

void c_mmu_fill_tables(mmu_tables_t *tables, uint32_t base_svc_stack_addr,
                       uint32_t base_irq_stack_addr);
void c_mmu_init(void);
int32_t c_mmu_map_4kb_page(mmu_tables_t *tables, uint32_t virt_addr,
                           uint32_t phys_addr, uint32_t l2_flags);
int32_t identity_map_region(mmu_tables_t *tables, uint32_t virt_addr,
                            uint32_t phys_addr, uint32_t size_in_kb);

#endif // __MMU_LIB_H__
