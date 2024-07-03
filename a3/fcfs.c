#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct threadinfo {
	int p_id;
	int arr_time;
	int burst_time;
	int waiting_time;
	int turn_around_time;
};

int main(int file_num, char *file[]) {
	struct threadinfo th[999];
	FILE *input=NULL;
	char *input_filename = file[1];

	//Checks if there are two parameters passed and if not stop the program
	if (file_num <= 1) {
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

    char line[255];

    int n = 0;
    int completion_time = 0;
    int total_turn_around = 0;
    int total_waiting = 0;

    printf("Thread ID	Arrival Time	Burst Time	 Completion Time	Turn-Around Time	Waiting Time\n");

    //Copies each line from input, stops when there are no more lines
    while (fgets(line, sizeof(line), input) != NULL) {
    	char *token;

    	//Get thread ID
    	token = strtok(line, ",");
    	th[n].p_id = atoi(token);

    	//Get arrival time
    	token = strtok(NULL, ",");
    	th[n].arr_time = atoi(token);

    	//Get burst times
    	token = strtok(NULL, ",");
    	th[n].burst_time = atoi(token);

    	//Calculate completion time, turn around time, and waiting time for each thread
    	completion_time += th[n].burst_time;
    	th[n].turn_around_time = completion_time - th[n].arr_time;
    	th[n].waiting_time = th[n].turn_around_time - th[n].burst_time;

    	//Calculate total turn around time and total waiting time for all threads
    	total_turn_around += th[n].turn_around_time;
    	total_waiting += th[n].waiting_time;

    	//Print all the thread information
    	printf("%d  %20d  %12d  %18d  %20d  %20d\n", th[n].p_id, th[n].arr_time, th[n].burst_time, completion_time, th[n].turn_around_time, th[n].waiting_time);

    	n++;
    }

    //Close the file
    fclose(input);

    //Calculate average turn around time and average waiting time for all threads
    float avg_turn_around = (float) total_turn_around / n;
    float avg_waiting = (float) total_waiting / n;

    printf("Average waiting time: %0.2f\n", avg_waiting);
    printf("Average turn-around time: %0.2f\n", avg_turn_around);

    return 0;

}
