# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -g

# Programs
PROGS = directory filecopy

# Default Target
all: $(PROGS)

# Rule for directory program
directory: directory.o
	$(CC) $(CFLAGS) -o directory directory.o

directory.o: directory.c
	$(CC) $(CFLAGS) -c directory.c

# Rule for filecopy program
filecopy: filecopy.o
	$(CC) $(CFLAGS) -o filecopy filecopy.o

filecopy.o: filecopy.c
	$(CC) $(CFLAGS) -c filecopy.c

# Clean rule to remove compiled files
clean:
	rm -f *.o $(PROGS)

# Run directory program
run-directory: directory
	./directory

# Run filecopy program
run-filecopy: filecopy
	./filecopy

# Phony targets
.PHONY: all clean run-directory run-filecopy
