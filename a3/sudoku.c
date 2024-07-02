#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 9

typedef struct {
    int row;
    int column;
} parameters;

int sudoku[N][N];
int results[N * 2 + N / 3 * N / 3];

void readSudokuFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fscanf(file, "%d", &sudoku[i][j]) != 1) {
                fprintf(stderr, "Error: Invalid Sudoku puzzle format\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}

void* check_row(void* param) {
    parameters* p = (parameters*) param;
    int row = p->row;
    int valid[N] = {0};

    for (int i = 0; i < N; i++) {
        int num = sudoku[row][i];
        if (num < 1 || num > 9 || valid[num - 1] == 1) {
            results[row] = 0;
            pthread_exit(0);
        }
        valid[num - 1] = 1;
    }

    results[row] = 1;
    pthread_exit(0);
}

void* check_column(void* param) {
    parameters* p = (parameters*) param;
    int column = p->column;
    int valid[N] = {0};

    for (int i = 0; i < N; i++) {
        int num = sudoku[i][column];
        if (num < 1 || num > 9 || valid[num - 1] == 1) {
            results[column + N] = 0;
            pthread_exit(0);
        }
        valid[num - 1] = 1;
    }

    results[column + N] = 1;
    pthread_exit(0);
}

void* check_subgrid(void* param) {
    parameters* p = (parameters*) param;
    int row = p->row;
    int column = p->column;
    int valid[N] = {0};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int num = sudoku[row + i][column + j];
            if (num < 1 || num > 9 || valid[num - 1] == 1) {
                results[row / 3 * 3 + column / 3 + 2 * N] = 0;
                pthread_exit(0);
            }
            valid[num - 1] = 1;
        }
    }

    results[row / 3 * 3 + column / 3 + 2 * N] = 1;
    pthread_exit(0);
}

int main() {
    const char* filename = "sample_in_sudoku.txt";
    readSudokuFromFile(filename);

    pthread_t threads[N * 2 + N / 3 * N / 3];
    parameters* data;

    for (int i = 0; i < N; i++) {
        data = (parameters*) malloc(sizeof(parameters));
        data->row = i;
        data->column = 0;
        pthread_create(&threads[i], NULL, check_row, data);

        data = (parameters*) malloc(sizeof(parameters));
        data->row = 0;
        data->column = i;
        pthread_create(&threads[i + N], NULL, check_column, data);
    }

    for (int i = 0; i < N; i += 3) {
        for (int j = 0; j < N; j += 3) {
            data = (parameters*) malloc(sizeof(parameters));
            data->row = i;
            data->column = j;
            pthread_create(&threads[2 * N + i + j / 3], NULL, check_subgrid, data);
        }
    }

    for (int i = 0; i < N * 2 + N / 3 * N / 3; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < N * 2 + N / 3 * N / 3; i++) {
        if (results[i] == 0) {
            printf("Sudoku solution is invalid.\n");
            return 0;
        }
    }

    printf("Sudoku solution is valid.\n");
    return 0;
}