#ifndef __CACHESIM_H
#define __CACHESIM_H
typedef unsigned int uint;
typedef unsigned long addr_t;
typedef unsigned long uint32_t;
typedef unsigned long long counter_t;
typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef unsigned char byte_t;


void cachesim_init(uint block_size, uint cache_size, uint ways);
void cachesim_access(addr_t physical_add, uint write);

void cachesim_print_stats(void);
void cachesim_destruct(void);

#endif
