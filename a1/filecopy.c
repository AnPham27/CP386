#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int file_num, char *files[]) {
	FILE *input=NULL, *output=NULL;
	char *input_filename = files[1];
	char *output_filename = files[2];

	//Checks if there are two parameters passed and if not stop the program
	if (file_num <= 2) {
		printf("Insufficient parameters passed.\n");
		return 1;
	}

	//Open input file for reading
	input = fopen(input_filename, "r");

	//If input file can't be found inform the user then stop the program
	if(input == NULL) {
		printf("Cannot open file: %s\n", input_filename);
		return 1;
	}

	//Open output file for writing
	output = fopen(output_filename, "w");

	char line[255];

	//Copies each line from input to output, stops when there are no more lines to copy
	while (fgets(line, sizeof(line), input) != NULL) {
		fprintf(output, "%s", line);
	}

	printf("Contents of %s copied to %s\n", input_filename, output_filename);

	//Close the input and output files
	fclose(input);
	fclose(output);

	return 0;
}

