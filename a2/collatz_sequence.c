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

void handleChildProcess(int *sequence) {
    for (int j = 0; j < MAX_LENGTH; j++) {
        //Print the current element
        printf("%d", sequence[j]);
        
        //Check if the end of the sequence is reached
        if (sequence[j] == 1) {
            printf("\n");
            break;
        } else {
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
        // Check if the correct number of arguments is passed
        fprintf(stderr, "Error: Insufficient parameters passed.\n");
        return EXIT_FAILURE;
    }

    // Open the file for reading
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error: Unable to open file");
        return EXIT_FAILURE;
    }

    int start_numbers[ARRAY_SIZE];
    int index = 0;
    int num;

    // Read numbers from the file and populate the array
    while (fscanf(file, "%d", &num) == 1 && index < ARRAY_SIZE) {
        start_numbers[index++] = num;
    }
    fclose(file);

    // Process each number in the array
    for (int i = 0; i < index; i++) {
        // Create shared memory for the Collatz sequence
        int *sequence = mmap(NULL, MAX_LENGTH * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sequence == MAP_FAILED) {
            fprintf(stderr, "Error: Failed to create shared memory.\n");
            return EXIT_FAILURE;
        }

        // Generate the Collatz sequence for the current number
        collatz_seq(start_numbers[i], sequence);

        // Fork a child process
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error: Failed to fork process");
            return EXIT_FAILURE;
        } else if (pid == 0) {
            // Child process
            printf("Child Process: Collatz sequence for %d: ", start_numbers[i]);
            handle_child_process(sequence);
            munmap(sequence, MAX_LENGTH * sizeof(int));
            return EXIT_SUCCESS;
        } else {
            // Parent process
            printf("Parent Process: Reading positive integer %d from file.\n", start_numbers[i]);
            wait(NULL); // Wait for child process to finish
            munmap(sequence, MAX_LENGTH * sizeof(int)); // Cleanup shared memory
        }
    }

    return EXIT_SUCCESS;
}
