#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cachesim.h"

counter_t accesses = 0, write_hits = 0, read_hits = 0, 
          write_misses = 0, read_misses = 0, writebacks = 0;

//struct declarations
struct line {
	uint tag;
	uint valid;
	uint dirty;
	counter_t since_used;
} myLine;

struct set {
    struct line *lines;
} mySet;

struct cache {
    uint cache_size, block_size, ways;
    uint num_lines;
    //struct set *sets; //each block will have n lines
    struct line *lines;
};


/* Use this method to initialize your cache data structure
* You should do all of your memory allocation here
*/
struct cache *myCache;
void cachesim_init(uint blocksize, uint cachesize, uint ways) 
{
	myCache = malloc(sizeof(myCache));
	myCache -> cache_size = cachesize;
	myCache -> block_size = blocksize;
	myCache -> ways = ways;
	// myCache -> sets = (struct set *)malloc((cachesize/(blocksize*ways)) * sizeof(mySet));
	// for (int i = 0; i < (cachesize/(blocksize*ways)); i++) {
	// 	myCache -> sets[i].lines = (struct line *)malloc(ways * sizeof(myLine));
	// 	for (int j = 0; j < ways; j++) {
	// 		myCache -> sets[i].lines[j].tag = -1; //prevents false hits with 0 tag
	// 		myCache -> sets[i].lines[j].valid = 0;
	// 		myCache -> sets[i].lines[j].dirty = 0;
	// 		myCache -> sets[i].lines[j].since_used = 0;
	// 	}
	// }
	myCache -> lines = (struct line *)malloc((cachesize/(blocksize)) * sizeof(myLine));
	for (int i = 0; i < (cachesize/(blocksize)); i++) {
		myCache -> lines[i].tag = -1; //prevents false hits with 0 tag
		myCache -> lines[i].valid = 0;
		myCache -> lines[i].dirty = 0;
		myCache -> lines[i].since_used = 0;
	}
}

/* Clean up dynamically allocated memory when the program terminates */
void cachesim_destruct()
{
	
	// for (int i = 0; i < ((myCache -> cache_size)/((myCache -> block_size)*(myCache -> ways))); i++) {
	// 	free(myCache -> sets[i].lines);
	// }
	// free(myCache -> sets);
	// free(myCache);
	// for (int i = 0; i < ((myCache -> cache_size)/((myCache -> block_size))); i++) {
	// 	free(myCache -> lines[i]);
	// }
	free(myCache -> lines);
	free(myCache);
}

/* Called on each access
* 'write' is a boolean representing whether a request is read(0) or write(1)
*/
void cachesim_access(addr_t physical_addr, uint write) 
{
	uint offset_bits = log2(myCache -> block_size);
	uint index_bits = log2((myCache -> cache_size)/((myCache -> block_size)*(myCache -> ways)));
	uint tag_bits = 4*8 - offset_bits - index_bits;
	uint tag = physical_addr >> offset_bits + index_bits;
	uint index = physical_addr << tag_bits >> (tag_bits + offset_bits);
	uint offset = physical_addr << (tag_bits + index_bits) >> (tag_bits + index_bits);
	uint bool_hit = 0; //boolean for hit
	accesses++;
	for (int i = 0; i < (myCache -> ways); i++) {
	 	//if (myCache -> sets[index].lines[i].valid == 1 && myCache -> sets[index].lines[i].tag == tag) {
		if (myCache -> lines[index*(myCache -> ways) + i].valid == 1 && myCache -> lines[index*(myCache -> ways) + i].tag == tag) {
			// bool_hit = 1;
			// for (int j = 0; j < (myCache -> ways); j++) {
			// 	if (j == i) {
			// 		myCache -> sets[index].lines[j].since_used = 0;
			// 	} else {
			// 		myCache -> sets[index].lines[j].since_used++;
			// 	}
			// }
			// if (write == 1) {
			// 	write_hits++;
			// 	myCache -> sets[index].lines[i].dirty = 1;
			// } else {
			// 	read_hits++;
			// }
			// break;
			bool_hit = 1;
			for (int j = 0; j < (myCache -> ways); j++) {
				if (j == i) {
					myCache -> lines[index*(myCache -> ways) + j].since_used = 0;
				} else {
					myCache -> lines[index*(myCache -> ways) + j].since_used++;
				}
			}
			if (write == 1) {
				write_hits++;
				myCache -> lines[index*(myCache -> ways) + i].dirty = 1;
			} else {
				read_hits++;
			}
			break;
	 	}
	}
	// if (bool_hit == 0) {
	// 	//find oldest way in set
	// 	int oldest = 0;
	// 	for (int i = 0; i < (myCache -> ways); i++) {
	// 		if (myCache -> sets[index].lines[i].since_used > myCache -> sets[index].lines[oldest].since_used) {
	// 			oldest = i;
	// 		}
	// 	}

	// 	//replace oldest
	// 	myCache -> sets[index].lines[oldest].tag = tag;
	// 	myCache -> sets[index].lines[oldest].valid = 1;
	// 	for (int j = 0; j < (myCache -> ways); j++) {
	// 		if (j == oldest) {
	// 			myCache -> sets[index].lines[j].since_used = 0;
	// 		} else {
	// 			myCache -> sets[index].lines[j].since_used++;
	// 		}
	// 	}
	// 	if (myCache -> sets[index].lines[oldest].dirty == 1) {
	// 		writebacks++;
	// 		myCache -> sets[index].lines[oldest].dirty = 0;
	// 	}
	// 	if (write == 1) {
	// 		write_misses++;
	// 		myCache -> sets[index].lines[oldest].dirty = 1;
	// 	} else {
	// 		read_misses++;
	// 	}


	// }
	if (bool_hit == 0) {
		//find oldest way in set
		int oldest = 0;
		for (int i = 0; i < (myCache -> ways); i++) {
			if (myCache -> lines[index*(myCache -> ways) + i].since_used > myCache -> lines[index*(myCache -> ways) + oldest].since_used) {
				oldest = i;
			}
		}

		//replace oldest
		// myCache -> lines[index*(myCache -> ways) + oldest].tag = tag;
		// myCache -> lines[index*(myCache -> ways) + oldest].valid = 1;
		for (int j = 0; j < (myCache -> ways); j++) {
			if (j == oldest) {
				myCache -> lines[index*(myCache -> ways) + j].since_used = 0;
			} else {
				myCache -> lines[index*(myCache -> ways) + j].since_used++;
			}
		}
		// if (myCache -> lines[index*(myCache -> ways) + oldest].dirty == 1) {
		// 	writebacks++;
		// 	myCache -> lines[index*(myCache -> ways) + oldest].dirty = 0;
		// }
		// if (write == 1) {
		// 	write_misses++;
		// 	myCache -> lines[index*(myCache -> ways) + oldest].dirty = 1;
		// } else {
		// 	read_misses++;
		// }


	}


}

/* You may not change this method in your final submission!!!!! 
*   Furthermore, your code should not have any extra print statements
*/
void cachesim_print_stats()
{
  printf("%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, read_hits, write_hits, 
                                                read_misses, write_misses, writebacks);
  // float out1 = (float)(read_misses + write_misses)/accesses;
  // float out2 = (float)read_misses/(read_hits + read_misses);
  // float out3 = (float)write_misses/(write_hits + write_misses);
  // printf("Overall: %f, Read: %f, Write: %f\n", out1, out2, out3);

  // printf("%d\n", writebacks*(myCache->block_size));

  // printf("%d %d\n", (read_misses + write_misses)*myCache->block_size, accesses*myCache->block_size);


}
