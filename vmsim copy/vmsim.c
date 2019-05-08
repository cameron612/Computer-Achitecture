#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmsim.h"
#include "assert.h"

counter_t accesses = 0, tlb_hits = 0,
 tlb_misses = 0, page_faults = 0, disk_writes = 0, shutdown_writes = 0;

byte_t mem[MEM_SIZE]; //static allocation of memory

struct FT {
  byte_t protected;
  byte_t mapped;
  uint32_t VPN;
} *FT;
struct PT {
  byte_t PPN;
  byte_t valid_dirty;
} *PT;

struct TLB {
  struct line *lines;
};
struct line {
  addr_t PPN;
  uint dirty;
  uint valid;
  uint32_t tag;
  counter_t since_used;
};

/* 0. Zero out memory 
*  1. Initialize your TLB (do not place FT or PT in TLB)
*  2. Create a frame table and place it into mem[]. 
*   - The frame table should reside in the first frame of memory
*   - The frame table should occupy no more than one frame, FTE size is up to you
*  3. Create a page table and place it into mem[]
*   - The page table will reside in the sequential frames of memory after the FT
*   - The page table should not reside in the same frame as the frame table
*   - Each PTE is limited to 2 bytes maximum
*  4. Mark all the frames holding the FT and PT as mapped and protected
*  
*/
struct TLB *myTLB;
void system_init() 
{
  //0. Zero out memory 
  for (int i = 0; i < MEM_SIZE; ++i) {
    mem[i] = 0;
  }
  //1. Initialize your TLB (do not place FT or PT in TLB)
  myTLB = (struct TLB *)malloc(sizeof(myTLB));
  myTLB -> lines = (struct line *)malloc(TLB_SIZE * sizeof(struct line));
  for (int i = 0; i < TLB_SIZE; ++i) {
    myTLB -> lines[i].PPN = -1;
    myTLB -> lines[i].dirty = 0;
    myTLB -> lines[i].valid = 0;
    myTLB -> lines[i].tag = -1;
    myTLB -> lines[i].since_used = 0;
  }
  //2. Create a frame table and place it into mem[]. 
  FT = (struct FT*)&mem[0];
  //3. Create a page table and place it into mem[]
  PT = (struct PT*)&mem[PAGE_SIZE];
  //4. Mark all the frames holding the FT and PT as mapped and protected
  for (int i = 0; i < 3; i++) {
    FT[i].mapped = 1;
    FT[i].protected = 1;
  }
}

/* At system shutdown, you need to write all dirty
* frames back to disk
* These are different from the dirty pages you write back on an eviction
* Count the number of dirty frames in the physical memory here and store it in
* shutdown_writes
* finally, return mem
*/
byte_t* system_shutdown()
{
  //update shutdown writes
  for (int i = 0; i < PT_NUM_ENTRIES; ++i) {
    if (PT[i].valid_dirty & 1 == 1) {
      shutdown_writes++;
    }
  }
  return mem;
}

/*
* Check the TLB for an entry
* returns HIT or MISS
* If HIT, returns the physcial address in paddr
* Updates the state of TLB based on write
* 
*/
status_t check_TLB(addr_t vaddr, uint write, addr_t* paddr)
{
  uint32_t VPN = vaddr >> 12;
  //*paddr = 0x00BEEF;
  for (int i = 0; i < TLB_SIZE; ++i) {
    if (myTLB -> lines[i].tag == VPN) {
      addr_t PPN = myTLB -> lines[i].PPN;
      //change to pointer when needed
      *paddr = (PPN << 12) | (vaddr & 4095);
      return HIT;
    }
  }
  //printf("%x\n", VPN);
  return MISS; 
}

/*
* Check the Page Table for an entry
* returns HIT or MISS
* If HIT, returns the physcial address in paddr
* Updates the state of Page Table based on write
*/
status_t check_PT(addr_t vaddr, uint write, addr_t* paddr)
{
  uint32_t VPN = (vaddr >> 12);
  //check validity
  if ((PT[VPN].valid_dirty) & 2 == 2) {
    uint PPN = PT[VPN].PPN << 1 | (PT[VPN].valid_dirty & 128) >> 7;
    //change to pointer when needed
    *paddr = (PPN << 12) | (vaddr & 4095);
    return HIT;
  }
  //printf("%x\n", VPN);
  return MISS; 
}

/*
* Updates the TLB
* If you hit in the tlb, you still need to update LRU status
* as well as dirty status:
*   Remember, if the dirty bit is newly set, you should go to memory and
*   mark the corresponding PTE dirty
* If you miss in the TLB, you need to replace:
*   First, check the TLB for any invalid entries
*   If one is found, install the vaddr mapping into it
*   Otherwise, consult an LRU replacement policy
*
* When a TLB entry is kicked out, what should you do? Anything?
*/
void update_TLB(addr_t vaddr, uint write, addr_t paddr, status_t tlb_access)
{
  if (tlb_access == HIT) {
    for (int i = 0; i < TLB_SIZE; ++i) {
      if (myTLB -> lines[i].tag == vaddr >> 12) {
        myTLB -> lines[i].since_used = 0;
        if (write == 1) {
          if (myTLB -> lines[i].dirty != 1) {
            //go to memory and mark the corresponding PTE dirty
            PT[vaddr >> 12].valid_dirty = PT[vaddr >> 12].valid_dirty | 1;
          }
          myTLB -> lines[i].dirty = 1;
        }
      } else {
        myTLB -> lines[i].since_used++;
      }
    }
  } else {
    uint inv = 0;
    //search for invalid
    for (int i = 0; i < TLB_SIZE; ++i) {
      if (myTLB -> lines[i].valid == 0) {
        //install vaddr maping
        myTLB -> lines[i].PPN = paddr >> 12;
        myTLB -> lines[i].tag = vaddr >> 12;
        myTLB -> lines[i].valid = 1;
        myTLB -> lines[i].since_used = 0;
        inv = 1;
      } else {
        myTLB -> lines[i].since_used++;
      }
    }
    //LRU replacement
    if (inv == 0) {
      int oldest = 0;
      for (int i = 0; i < TLB_SIZE; i++) {
        if (myTLB -> lines[i].since_used > myTLB -> lines[oldest].since_used) {
          oldest = i;
        }
      }

      //replace oldest
      myTLB -> lines[oldest].tag = vaddr >> 12;
      myTLB -> lines[oldest].valid = 1;
      myTLB -> lines[oldest].since_used = 0;
      myTLB -> lines[oldest].dirty = 0;
    }
  }
}


/*
* Called on a page fault
* First, search the frame table, starting at fte 0 and iterating linearly through all 
* the frame table entries looking for unmapped unprotected frame.
* If one is found, use it.
* Otherwise, choose a frame randomly using rand(). Consider all frames possible in rand(),
* even the frame table and page table themselves.
* If the number lands on a protected frame, roll a random number again - repeat until you land
* on an unprotected frame, then replace that frame. 
* If the frame being replaced is dirty, increment disk_writes
* If the frame being accessed is in the TLB, mark it invalid in the TLB (here, not in update_TLB)
* Update the frame table with the the VPN as well
*
* returns the PPN of the new mapped frame
*/
uint32_t page_fault(addr_t vaddr, uint write)
{
  page_faults++;
  //4) We check the Frame Table. The index of the frame table is a PHYSICAL page number (Physical frame number) and what's inside the table is whether that physical page number is MAPPED, if it's PROTECTED, and the VPN that's mapped to it if it's mapped.
  uint32_t VPN = vaddr >> 12;
 

  //5) We go through each index of the frame table until we find an entry that is NOT mapped. Since Physical page number 0 is taken by the frame table, 1 and 2 are taken by the page table, the first one we find not mapped is index 3 = Physical page number 3. 
  for (int i = 0; i < FT_NUM_ENTRIES; i++) {
    //6) When we find an entry that's not mapped, we mark it as mapped, update it's VPN,  and return that index (physical page number).
    if (FT[i].mapped == 0 && FT[i].protected == 0) {
      FT[i].mapped = 1;
      FT[i].protected = 0;
      FT[i].VPN = VPN;
      return i;
    }
  }
  //In step 5, if all the physical frames are mapped we choose a RANDOM NON PROTECTED physical page number, invalidate the already existing VPN to PFN in the page table, update the VPN in the frame table,  and return that physical frame number to step 7. 
  //this is where disk writes gets incremented
  uint idx = rand() % FT_NUM_ENTRIES;
  //idx will be new PPN
  while (1) {
    if (FT[idx].protected == 0) {
      disk_writes++;
      FT[idx].VPN = VPN;
      //invalidate PTE
      PT[VPN].valid_dirty = (PT[vaddr >> 12].valid_dirty >> 2) << 2;
      return idx;
    } else {
      idx = rand() % FT_NUM_ENTRIES;
    }
  }
}


/* Called on each access 
* If the access is a write, update the memory in mem, return data
* If the access is a read, return the value of memory at the physical address
* Perform accessed in the following order
* Address -> read TLB -> read PT -> Update TLB -> Read Memory
*/
byte_t memory_access(addr_t vaddr, uint write, byte_t data) 
{
  accesses++;
  addr_t paddr = 0;
  // First, we check the TLB
  status_t tlbAccess = check_TLB(vaddr, write, &paddr);
  // Do memory system stuff
  if (tlbAccess == HIT) {
    tlb_hits++;
  } else {
    tlb_misses++;
    status_t ptAccess = check_PT(vaddr, write, &paddr);
    
    if (ptAccess == HIT) {
      if (write == 1) {
        PT[vaddr >> 12].valid_dirty = (PT[vaddr >> 12].valid_dirty >> 2) << 2 | 3;
      }
      //update TLB
      
    } else {
      //assigns PPN starting at 3
      if (write == 1) {
        PT[vaddr >> 12].valid_dirty = (PT[vaddr >> 12].valid_dirty >> 2) << 2 | 3;
      } else {
        PT[vaddr >> 12].valid_dirty = (PT[vaddr >> 12].valid_dirty >> 2) << 2 | 2;
      }
      uint32_t PPN = page_fault(vaddr, write);
      paddr = PPN << 12 | vaddr >> 12;
      //EXTEND TO ALL 9 BITS
      if (PPN > 255) {
        PT[vaddr >> 12].PPN = PPN >> 1;
        PT[vaddr >> 12].valid_dirty = PT[vaddr >> 12].valid_dirty | (PPN & 1) << 7;
      } else {
        PT[vaddr >> 12].PPN = PPN;
      }

    }
  }

  update_TLB(vaddr, write, paddr, tlbAccess); //update TLB after each access

  if(write) mem[paddr] = data; //Update mem on write 

  return mem[paddr];

}

void print_FT() {
  printf("FT:\n");
      for (int i = 0; i < PAGE_SIZE; i += 8) {
        printf("FT Entry #%d: ", i/8);
        printf("mapped: %x protected: %x VPN: %x%x%x%x%x%x\n", mem[i+1], mem[i],
          mem[i+2], mem[i+3], mem[i+4], mem[i+5], mem[i+6], mem[i+7]);
      }
}
void print_PT() {
  printf("\nPT:\n");
      for (int i = PAGE_SIZE; i < 3 * PAGE_SIZE; i += 2) {
        printf("PT Entry #%d: ", (i - PAGE_SIZE)/2);
        printf("PPN: %x VD: %x\n", mem[i], mem[i + 1]);
      }
}

/* You may not change this method in your final submission!!!!! 
*   Furthermore, your code should not have any extra print statements
*/
void vm_print_stats() 
{
  printf("%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, tlb_hits, tlb_misses, page_faults, disk_writes, shutdown_writes);
}

