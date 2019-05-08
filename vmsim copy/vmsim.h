#ifndef __VMSIM_H
#define __VMSIM_H
typedef unsigned int uint;
typedef unsigned long addr_t;
typedef unsigned int uint32_t; //
typedef unsigned long long counter_t;
typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef unsigned char byte_t;

typedef enum status_t {MISS, HIT} status_t;

void system_init();
byte_t* system_shutdown();
status_t check_TLB(addr_t vaddr, uint write, addr_t* paddr);
status_t check_PT(addr_t vaddr, uint write, addr_t* paddr);
void update_TLB(addr_t vaddr, uint write, addr_t paddr, status_t tlb_access);
uint32_t page_fault(addr_t vaddr, uint write);
byte_t memory_access(addr_t vaddr, uint write, byte_t data) ;
void vm_print_stats();
void print_FT();
void print_PT();

#define MEM_SIZE (1 << 21)
#define TLB_SIZE 5
#define PAGE_SIZE (1 << 12)
#define FT_NUM_ENTRIES (1 << 9)
#define PT_NUM_ENTRIES (1 << 12)

#endif
