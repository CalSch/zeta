#include "stuff.h"

#define MAX_BLOCKS 256
#define HEAP_SIZE 4096

struct mem_block_t {
	u16 size;
	void* ptr;
	u8 free; // TODO: how can this be compacted?
	u16 next; // index of next block, might need to be a u16 later. if next==0 then this block is the end
};

extern struct mem_block_t block_table[MAX_BLOCKS];
extern u8 heap[HEAP_SIZE];

void print_bt(void);

void setup_malloc(void);
void* malloc(u16 size);
void free(void* ptr);
void* realloc(void* ptr, u16 newsize);
