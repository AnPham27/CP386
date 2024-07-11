#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 1000

typedef struct {
	int start;
	int size;
	bool free;
	int process_id;
}Block;

Block memory[MAX];

int blockCount = 1;

void init_memory() {
	memory[0].start = 0;
	memory[0].size = MAX;
	memory[0].free = true;
	memory[0].process_id = -1;
}

int allocate_memory(int requestSize, int process_id) {
	int bestFitIndex = -1;
	int bestFitSize = MAX + 1;
	// Find the smallest hole big enough to accommodate the request size
	for (int i = 0; i < blockCount; i++) {
		if (memory[i].free && memory[i].size >= requestSize && memory[i].size < bestFitSize) {
			bestFitIndex = i;
			bestFitSize = memory[i].size;
		}
	}

	// If there is no hole big enough, print an error message
	if (bestFitIndex == -1) {
		printf("No hole of sufficient size \n");
		return -1;
	}

	Block *block = &memory[bestFitIndex];

	// If hole is bigger than request size, split hole into two where one hole is equal to the request size
	if (block->size > requestSize) {
		for (int j = blockCount; j > bestFitIndex; j--) {
			memory[j] = memory[j-1];
		}
		memory[bestFitIndex + 1].start = block->start + requestSize;
		memory[bestFitIndex + 1].size = bestFitSize - requestSize;
		memory[bestFitIndex + 1].free = true;
		memory[bestFitIndex + 1].process_id = -1;
		blockCount++;
	}
	block->size = requestSize;
	block->free = false;
	block->process_id = process_id;

	return block->start;
}

int release_memory(int address) {
	// Find block at address and release the memory
	for (int i = 0; i < blockCount; i++) {
		if (memory[i].start == address && !memory[i].free) {
			memory[i].free = true;
			// If there is a free block next to it, fuse both blocks together
			if (i > 0 && memory[i-1].free) {
				memory[i-1].size += memory[i].size;
				for (int j = i; j < blockCount - 1; j++) {
					memory[j] = memory[j+1];
				}
			}
			if (i < blockCount -1 && memory[i+1].free) {
				memory[i+1].size += memory[i].size;
				for (int j = i; j > 0; j--) {
					memory[j] = memory[j-1];
				}
			}
		}
	}
}

