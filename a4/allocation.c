#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX 1048576

typedef struct {
    int start;
    int size;
    bool free;
    int process_id;
} Block;

Block memory[MAX];
int blockCount = 1;

void init_memory(int size) {
    memory[0].start = 0;
    memory[0].size = size;
    memory[0].free = true;
    memory[0].process_id = -1;
}

// User inputs process number like: P0, P1, P2 
int extract_process_id(const char* process_id_str) {
    if (process_id_str[0] == 'P') {
        return atoi(process_id_str + 1);
    }
    return -1;
}

// Allocating memory based on algorithm
int allocate_memory(int requestSize, int process_id, char strategy) {
    int index = -1;
    
    if (strategy == 'F') {
        // First Fit
        for (int i = 0; i < blockCount; i++) {
            if (memory[i].free && memory[i].size >= requestSize) {
                index = i;
                break;
            }
        }
    } else if (strategy == 'B') {
        // Best Fit
        int bestFitSize = MAX + 1;  // Initialize to a large value
        for (int i = 0; i < blockCount; i++) {
            if (memory[i].free && memory[i].size >= requestSize && memory[i].size < bestFitSize) {
                index = i;
                bestFitSize = memory[i].size;
            }
        }
    } else if (strategy == 'W') {
        // Worst Fit
        int worstFitSize = -1;
        for (int i = 0; i < blockCount; i++) {
            if (memory[i].free && memory[i].size >= requestSize && memory[i].size > worstFitSize) {
                index = i;
                worstFitSize = memory[i].size;
            }
        }
    }

    // Check if a suitable block was found
    if (index == -1) {
        printf("No hole of sufficient size\n");
        return -1;
    }

    Block *block = &memory[index];

    // Allocate memory and split block if necessary
    if (block->size > requestSize) {
        for (int j = blockCount; j > index + 1; j--) {
            memory[j] = memory[j - 1];
        }
        memory[index + 1].start = block->start + requestSize;
        memory[index + 1].size = block->size - requestSize;
        memory[index + 1].free = true;
        memory[index + 1].process_id = -1;
        blockCount++;
    }

    block->size = requestSize;
    block->free = false;
    block->process_id = process_id;

    return block->start;
}

// Releasing memory that has been allocated to a process
int release_memory(int process_id) {
    printf("Releasing memory for process P%d\n", process_id);
    for (int i = 0; i < blockCount; i++) {
        if (memory[i].process_id == process_id && !memory[i].free) {
            memory[i].free = true;
            if (i > 0 && memory[i - 1].free) {
                memory[i - 1].size += memory[i].size;
                for (int j = i; j < blockCount - 1; j++) {
                    memory[j] = memory[j + 1];
                }
                blockCount--;
                i--;
            }
            if (i < blockCount - 1 && memory[i + 1].free) {
                memory[i].size += memory[i + 1].size;
                for (int j = i + 1; j < blockCount - 1; j++) {
                    memory[j] = memory[j + 1];
                }
                blockCount--;
            }
            return 0;
        }
    }
    return -1;
}

// Compact the set of holes into one larger hole 
void compact_memory() {
    int freeIndex = 0;
    for (int i = 0; i < blockCount; i++) {
        if (!memory[i].free) {
            if (i != freeIndex) {
                memory[freeIndex] = memory[i];
                memory[freeIndex].start = (freeIndex == 0) ? 0 : memory[freeIndex - 1].start + memory[freeIndex - 1].size;
            }
            freeIndex++;
        }
    }
    memory[freeIndex].start = (freeIndex == 0) ? 0 : memory[freeIndex - 1].start + memory[freeIndex - 1].size;
    memory[freeIndex].size = MAX - memory[freeIndex].start;
    memory[freeIndex].free = true;
    memory[freeIndex].process_id = -1;
    blockCount = freeIndex + 1;
}

// Output the results of status memory
void status() {
    int allocatedMemory = 0;
    int freeMemory = 0;

    printf("Partitions [Allocated memory= ");
    for (int i = 0; i < blockCount; i++) {
        if (!memory[i].free) {
            allocatedMemory += memory[i].size;
        } else {
            freeMemory += memory[i].size;
        }
    }
    printf("%d]:\n", allocatedMemory);

    for (int i = 0; i < blockCount; i++) {
        if (!memory[i].free) {
            printf("Address [%d:%d] Process P%d\n", memory[i].start, memory[i].start + memory[i].size - 1, memory[i].process_id);
        }
    }

    printf("\nHoles [Free memory= %d]:\n", freeMemory);

    for (int i = 0; i < blockCount; i++) {
        if (memory[i].free) {
            printf("Address [%d:%d] len = %d\n", memory[i].start, memory[i].start + memory[i].size - 1, memory[i].size);
        }
    }
}

// Implementation
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <initial_memory_size>\n", argv[0]);
        return 1;
    }

    int initial_memory_size = atoi(argv[1]);
    if (initial_memory_size <= 0) {
        printf("Invalid memory size\n");
        return 1;
    }

    init_memory(initial_memory_size);
    printf("Here, the Best Fit approach has been implemented and the allocated %d bytes of memory.\n", initial_memory_size);

    char command[20];
    while (true) {
        printf("allocator> ");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            char process_id_str[10];
            int size;
            char strategy;
            scanf("%s %d %c", process_id_str, &size, &strategy);
            int process_id = extract_process_id(process_id_str);
            if (process_id == -1) {
                printf("Invalid process ID format\n");
                continue;
            }
            int address = allocate_memory(size, process_id, strategy);
            if (address != -1) {
                printf("Successfully allocated %d to process %s\n", size, process_id_str);
            } else {
                printf("Failed to allocate %d to process %s\n", size, process_id_str);
            }
        } else if (strcmp(command, "RL") == 0) {
            char process_id_str[10];
            scanf("%s", process_id_str);
            int process_id = extract_process_id(process_id_str);
            if (process_id == -1) {
                printf("Invalid process ID format\n");
                continue;
            }
            if (release_memory(process_id) == 0) {
                printf("Successfully released memory for process %s\n", process_id_str);
            } else {
                printf("Failed to release memory for process %s\n", process_id_str);
            }
        } else if (strcmp(command, "C") == 0) {
            compact_memory();
            printf("Compaction process is successful\n");
        } else if (strcmp(command, "Status") == 0) {
            status();
        } else if (strcmp(command, "Exit") == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }

    return 0;
}
