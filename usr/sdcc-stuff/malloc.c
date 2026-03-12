#include "malloc.h"

struct mem_block_t block_table[MAX_BLOCKS];

u8 heap[HEAP_SIZE];

void setup_malloc(void) {
	/* block_table[0] = (struct mem_block_t){.size=0, .ptr=&heap, .free=1, .next=0}; */
	block_table[0].size = HEAP_SIZE;
	block_table[0].ptr = &heap;
	block_table[0].free = 1;
	block_table[0].next = 0;
}

void print_b(struct mem_block_t b) {
	PRINTd(b.size);
	PRINTd(b.ptr);
	PRINTd(b.free);
	PRINTd(b.next);
}
void print_bt(void) {
	for (int i=0;i<5;i++) {
		puts("\nblock ");putdec(i);newline();
		print_b(block_table[i]);
	}
}

// find the first block with size 0
u16 new_block_idx(void) {
	u16 idx = 0;
	while (block_table[idx].size != 0)
		idx++;
	return idx;
}

void* malloc(u16 size) {
	struct mem_block_t* block = &block_table[0];

	// find a block
	while (block->next != 0) {
		if (block->size >= size && block->free)
			break;
		block = &block_table[block->next];
	}

	// ts shouldnt happen. but might if it cant find a block
	if (!block->free) {
		puts("==== block aint free! ====\n");
		print_b(*block);
		return NULL;
	}

	// we now have a good block, need to split it
	
	if (block->size == size) {
		// actually, we don't need to split it because it's the right size
		block->free = 0; // mark as used
		return block->ptr;
	}

	/* newline(); */
	/* puts("==== malloc ====\nfound block:\n"); */
	/* print_b(*block); */
	/* newline(); */

	u16 new_idx = new_block_idx();

	struct mem_block_t split_block;
	// the new block is to the right of the current block and...
	split_block.size = block->size - size; // takes the remaining size,
	split_block.ptr = (void*)((u16)block->ptr + size); // has the same pointer, but offset by the size to allocate,
	split_block.free = 1; // is free,
	split_block.next = block->next; // and points to what *was* the next block

	// the current block...
	block->next = new_idx; // points to the split block,
	block->size = size; // has the correct size,
	// keeps its ptr,
	block->free = 0; // and is not free
	
	block_table[new_idx] = split_block;

	// TODO: make an actuall error and move the check
	if (split_block.ptr - &heap > HEAP_SIZE) {
		__asm
			out (4), A
		__endasm;
	}

	return block->ptr;

}

u16 find_block(void* ptr) {
	struct mem_block_t* block = &block_table[0];
	u16 idx = 0;
	u16 counter = 0;
	while (block->ptr != ptr && counter<MAX_BLOCKS) {
		idx = block->next;
		block = &block_table[block->next];
		counter++;
	}
	return idx;
}

// TODO: combine adjacent free blocks
void free_block(u16 idx) {
	block_table[idx].free = 1;
	/* puts("==== freeing ");putdec(idx);puts(" ====\n"); */
	/* print_b(block_table[idx]); */
}

inline void free(void* ptr) {
	u16 idx = find_block(ptr);
	free_block(idx);
}

void* realloc(void* ptr, u16 newsize) {
	// Simple implementation always reserves more memory
	// and has no error checking
	u16 old_idx = find_block(ptr);

	void *newptr = malloc(newsize);
	u16 oldsize = block_table[old_idx].size;
	if (ptr) memcpy(newptr, ptr, newsize < oldsize ? newsize : oldsize);
	free_block(old_idx);

	return newptr;
}



