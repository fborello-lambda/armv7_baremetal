#ifndef __MMU_LIB_H__
#define __MMU_LIB_H__

#include <stdint.h>

// Paging Descriptor flags
#define L1_TYPE_COARSE_TABLE 0x1
// L2 flags (small page, AP=RW, cacheable, bufferable, executable)
#define L2_DEFAULT_FLAGS 0x32

#define ERROR_L1_INDEX_OOR -1
#define ERROR_L2_INDEX_OOR -2
#define ERROR_L2_IN_USE -3
#define PAGING_SUCCESS 0

void c_mmu_init(uint32_t *ttbr0);
int32_t c_mmu_map_4kb_page(uint32_t *ttbr0, uint32_t virt_addr,
                           uint32_t phys_addr, uint32_t l2_flags);
int32_t identity_map_region(uint32_t *ttbr0, uint32_t virt_addr,
                            uint32_t phys_addr, uint32_t size_in_kb);

#endif // __MMU_LIB_H__
