
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define SHARED_MEMORY_SIZE 4096

// Function to write the output to the file
void writeOutput(char *command, char *output) {
    FILE *fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}

int main(int file_num, char *files[]) {

	char *input = files[1];

	// Checks if there are two parameters passed and if not stop the program
	if (file_num < 2) {
		printf("Insufficient parameters passed.\n");
		exit(1);
	}

	// Create shared memory
	int shm_fd = shm_open("/my_shared_mem", O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		perror("shm_open");
		exit(1);
	}

	// Set size of shared memory
	if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1) {
		perror("ftruncate");
		exit(1);
	}

	// Map shared memory
	char *shared_mem = mmap(0, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shared_mem == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	// Fork the first child process to read file
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		// Read the file and write to shared memory
		FILE *input_file = fopen(input, "r");
		if (input_file == NULL) {
			perror("fopen");
			exit(1);
		}
		fread(shared_mem, sizeof(char), SHARED_MEMORY_SIZE, input_file);
		fclose(input_file);
		exit(0);
	} else {
		// Wait for the child to finish
		wait(NULL);

		// Read commands from shared memory into allocated array
		char *commands = strdup(shared_mem);
		if (commands == NULL) {
			perror("strdub");
			exit(1);
		}

		// Split commands into lines
		char *command = strtok(commands, "\n");

		while (command != NULL) {
			int pipefd[2];
			if (pipe(pipefd) == -1) {
				perror("pipe");
				exit(1);
			}


			pid_t exec_pid = fork();
			if (exec_pid < 0) {
				perror("fork");
				exit(1);
			} else if (exec_pid == 0) {
				// Execute the command
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
				dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe

				char *args[] = {"/bin/sh", "-c", command, NULL};
				execvp(args[0], args);
				perror("execvp");
				exit(1);

			} else {
				close(pipefd[1]);

				char buffer[4096];
				ssize_t count;
				char output[4096] = {0};

				while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
					buffer[count] = '\0'; // Null-terminate the buffer
					strcat(output, buffer);
				}

				close(pipefd[0]);
				writeOutput(command, output);

				wait(NULL); // Wait for the exec child to finish
			}

			command = strtok(NULL, "\n");
			char *end;

			if (command != NULL) {
			    // Trim trailing space
			    end = command + strlen(command) - 1;
			    while (end > command && isspace((unsigned char)*end)) end--;

			    // Write new null terminator character
			    *(end + 1) = '\0';
			}
		}

		// Clean up
		free(commands);
		munmap(shared_mem, SHARED_MEMORY_SIZE);
		shm_unlink("/my_shared_mem");

	}


	return 0;
}

