#define _POSIX_C_SOURCE 200809L
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

void child_process(int *sequence) {
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
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, MAX_LENGTH * sizeof(int)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    int *shm_ptr = mmap(0, MAX_LENGTH * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    return shm_ptr;

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Not enough parameters passed");
        return EXIT_FAILURE;
    }
    
    FILE *file = fopen(argv[1], "r");      // open file to read start numbers from
    if (file == NULL) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    int startNumbers[ARRAY_SIZE];
    int index = 0;
    int num;

    while (fscanf(file, "%d", &num) == 1 && index < ARRAY_SIZE) {
        startNumbers[index++] = num;    //populate array with every number from the file
    }
    //close file
    fclose(file);

    for (int i = 0; i < index; i++) {
        //for loop through array generate collatz sequence for each number
        // create shared memory object and check if creation was successful
        int *sequence = shared_memory_object();

        if (sequence == NULL) {
            fprintf(stderr, "Failed to create shared memory object.\n");
            return EXIT_FAILURE;
        }

        collatz_seq(startNumbers[i], sequence);

        //Fork child process
        pid_t pid = fork();
        if (pid == -1) {
            perror("Failed to fork process");
            return EXIT_FAILURE;
        } else if (pid == 0) {  //child process
            printf("Child Process: The generated collatz sequence is ");
            child_process(sequence);
            munmap(sequence, MAX_LENGTH * sizeof(int));
            return EXIT_SUCCESS;
        } else {    //parent process
            printf("Parent Process: The positive integer read from file is %d\n", startNumbers[i]);
            wait(NULL);     //Wait for child process to finish
        }

        // Clean up
        munmap(sequence, MAX_LENGTH * sizeof(int));
    }

    //remove shared memory object
    shm_unlink(SHM_NAME);



    return 0;
}
