#include <stdint.h>

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1 << PAGE_SHIFT)
#define PFN_PRESENT     (1ull << 63)
#define PFN_PFN         ((1ull << 55) - 1)
#define PHY_RAM         0x80000000

void* mem_map( const char* dev, size_t offset, size_t size );
uint32_t page_offset(uint32_t addr);
uint64_t gva_to_gfn(void *addr);
uint64_t gva_to_gpa(void *addr);
