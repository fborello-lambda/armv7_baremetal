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
#define L2_TABLES_PER_TASK 8 // L2 tables per task

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

void c_mmu_fill_tables(mmu_tables_t *tables, uint32_t task_id);
void c_mmu_init(void);
int32_t c_mmu_map_4kb_page(mmu_tables_t *tables, uint32_t virt_addr,
                           uint32_t phys_addr, uint32_t l2_flags);
int32_t map_region(mmu_tables_t *tables, uint32_t virt_addr, uint32_t phys_addr,
                   uint32_t size_in_bytes);
// Memory Map
#define GET_SYMBOL_VALUE(sym) ((uint32_t) & (sym))
// KERNEL sections
extern uint32_t _KERNEL_TEXT_LMA, _KERNEL_TEXT_VMA, _KERNEL_TEXT_PHY;
extern uint32_t _KERNEL_DATA_LMA, _KERNEL_DATA_VMA, _KERNEL_DATA_PHY;
extern uint32_t _KERNEL_RODATA_LMA, _KERNEL_RODATA_VMA, _KERNEL_RODATA_PHY;
extern uint32_t _KERNEL_BSS_VMA, _KERNEL_BSS_PHY;
extern uint32_t _KERNEL_STACK;

// TASK0 sections
extern uint32_t _TASK0_TEXT_LMA, _TASK0_TEXT_VMA, _TASK0_TEXT_PHY;

// TASK1 sections
extern uint32_t _TASK1_TEXT_LMA, _TASK1_TEXT_VMA, _TASK1_TEXT_PHY;
extern uint32_t _TASK1_DATA_LMA, _TASK1_DATA_VMA, _TASK1_DATA_PHY;
extern uint32_t _TASK1_RODATA_LMA, _TASK1_RODATA_VMA, _TASK1_RODATA_PHY;
extern uint32_t _TASK1_BSS_VMA, _TASK1_BSS_PHY;
extern uint32_t _TASK1_STACK, _TASK1_STACK_PHY;
extern uint32_t _TASK1_RAREA_START_VMA, _TASK1_RAREA_END_VMA,
    _TASK1_RAREA_START_PHY;
#define TASK1_RAREA_SIZE_B                                                     \
  ((uint32_t)(GET_SYMBOL_VALUE(_TASK1_RAREA_END_VMA) -                         \
              GET_SYMBOL_VALUE(_TASK1_RAREA_START_VMA) + 1))

// TASK2 sections
extern uint32_t _TASK2_TEXT_LMA, _TASK2_TEXT_VMA, _TASK2_TEXT_PHY;
extern uint32_t _TASK2_DATA_LMA, _TASK2_DATA_VMA, _TASK2_DATA_PHY;
extern uint32_t _TASK2_RODATA_LMA, _TASK2_RODATA_VMA, _TASK2_RODATA_PHY;
extern uint32_t _TASK2_BSS_VMA, _TASK2_BSS_PHY;
extern uint32_t _TASK2_STACK, _TASK2_STACK_PHY;
extern uint32_t _TASK2_RAREA_START_VMA, _TASK2_RAREA_END_VMA,
    _TASK2_RAREA_START_PHY;
#define TASK2_RAREA_SIZE_B                                                     \
  ((uint32_t)(GET_SYMBOL_VALUE(_TASK2_RAREA_END_VMA) -                         \
              GET_SYMBOL_VALUE(_TASK2_RAREA_START_VMA) + 1))

// Declare SIZE to get their value from the address with the GET_SYMBOL_VALUE
// macro
extern uint8_t _KERNEL_TEXT_SIZE;
extern uint8_t _KERNEL_DATA_SIZE;
extern uint8_t _KERNEL_RODATA_SIZE;
extern uint8_t _KERNEL_BSS_SIZE;
extern uint8_t _KERNEL_STACK_SIZE;
extern uint8_t _TASK0_TEXT_SIZE;
extern uint8_t _TASK1_TEXT_SIZE;
extern uint8_t _TASK1_DATA_SIZE;
extern uint8_t _TASK1_RODATA_SIZE;
extern uint8_t _TASK1_BSS_SIZE;
extern uint8_t _TASK1_STACK_SIZE;
extern uint8_t _TASK2_TEXT_SIZE;
extern uint8_t _TASK2_DATA_SIZE;
extern uint8_t _TASK2_RODATA_SIZE;
extern uint8_t _TASK2_BSS_SIZE;
extern uint8_t _TASK2_STACK_SIZE;

#endif // __MMU_LIB_H__
