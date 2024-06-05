#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define MAX_LENGTH 1000
#define SHM_NAME "/collatz_shm"
#define ARRAY_SIZE 100
#define PARAMETERS 2



int collatz_seq(int number, int *sequence) {
    if (number <= 0 || sequence == NULL) {
        return -1; // Indicate an error
    }

    int i = 0;

    //generate Collatz sequence until we reach 1 (end of sequence)
    while (number != 1) {
        sequence[i++] = number;

        //update number based on whether it is even or odd
        if (number % 2 == 0) {
            number /= 2;
        } else {
            number = 3 * number + 1;
        }
    }

    //end of Collatz sequence, add 1
    sequence[i++] = 1;

    return i; //eeturn the length of the sequence
}

void handleChildProcess(int *sequence) {
    for (int j = 0; j < MAX_LENGTH; j++) {
        //Print the current element
        printf("%d", sequence[j]);
        
        //Check if the end of the sequence is reached
        if (sequence[j] == 1) {
            printf("\n");
            break;
        }
        else {
            printf(" ");
        }
    }
}

int *shared_memory_object() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1) {
        printf("Error creating shared memory.\n");
        exit(1);
    }

    ftruncate(shm_fd, MAX_LENGTH * sizeof(int));
    int *sequence = mmap(NULL, MAX_LENGTH * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    return sequence;

    if (sequence == MAP_FAILED) {
          printf("Error mapping shared memory\n");
          exit(1);
     }

     return sequence;
}


int main(int argc, char *argv[]) {
    if (argc != PARAMETERS)
    {
        fprintf(stderr, "Insufficient parameters passed.\n");
        return EXIT_FAILURE;
    }

    // open file to read start numbers from
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // array to store start numbers
    int startNumbers[ARRAY_SIZE];
    int index = 0;
    int num;

    // populate startNumbers array with every number from the file
    while (fscanf(fp, "%d", &num) == 1 && index < ARRAY_SIZE)
    {
        startNumbers[index++] = num;
    }
    fclose(fp);

    // iterate through startNumbers array and generate collatz sequence for each number
    for (int i = 0; i < index; i++)
    {
        // create shared memory object and check if creation was successful
        int *sequence = createSharedMemoryObject();
        if (sequence == NULL)
        {
            fprintf(stderr, "Failed to create shared memory object.\n");
            return EXIT_FAILURE;
        }

        // generate collatz sequence for number "i" and store it inside the shared memory object
        collatz_seq(startNumbers[i], sequence);

        // create a child process
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("Error forking process");
            return EXIT_FAILURE;
        }
        else if (pid == 0)
        {
            // child process => print collatz sequence
            printf("Child Process: The generated collatz sequence is ");
            handleChildProcess(sequence);
            munmap(sequence, MAX_LENGTH * sizeof(int));
            return EXIT_SUCCESS;
        }
        else
        {
            // parent process => wait for child process to finish executing
            printf("Parent Process: The positive integer read from file is %d\n", startNumbers[i]);
            wait(NULL);
        }

        // cleanup shared memory
        munmap(sequence, MAX_LENGTH * sizeof(int));
    }

    // remove shared memory object
    shm_unlink(SHM_NAME);

    return EXIT_SUCCESS;
}