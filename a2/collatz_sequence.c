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



void sequence(int n, int *seq, ) {
    int i = 0;
    //Base case: if number reaches 1, add it to the sequence and return
    if (n == 1) {
        seq[i] = 1;
        return;
    }
    //Add the current number to the sequence
    seq[i] = n;

    //Recursive case: determine the next number in the sequence
    if (n % 2 == 0) {
        seqence(n / 2, seq, i + 1);
    }
    else {
        seqence(3 * n + 1, seq, i + 1);
    }
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
        createCollatzSequence(startNumbers[i], sequence);

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