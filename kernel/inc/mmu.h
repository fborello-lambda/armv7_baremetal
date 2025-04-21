#ifndef __MMU_LIB_H__
#define __MMU_LIB_H__

#include <stdint.h>

// Paging Descriptor flags
#define L1_TYPE_COARSE_TABLE 0x1
// L2 flags (small page, AP=RW, cacheable, bufferable, executable)
#define L2_FLAGS 0x32

void c_mmu_init();
int32_t c_mmu_map_4kb_page(uint32_t virt_addr, uint32_t phys_addr);
int32_t identity_map_region(uint32_t virt_addr, uint32_t phys_addr,
                            uint32_t size_in_kb);

#endif // __MMU_LIB_H__
