/*
    Group: 24
    Course: SOFE3950U - Operating Systems
    Lab: Lab 3 - Sudoku Validator
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9

int sudoku[SIZE][SIZE]; // sudoku array

// struct holds parameters passed to the thread functions
typedef struct {
    int row;
    int column;
    int index;
    int *results;
} parameters; // parameters is the "alias" of the struct (user defined group of variables)

/* 
    Thread function that returns a null pointer and checks the given row for validity.
        - Sets an index of the results array to 1 corresponding to the index passed through the struct.
        - Leaves results array index as 0 if number is found to be out of bounds or repeated.
*/
void *check_row(void *arg) {
    parameters *p = (parameters *)arg; // copy of arg to ensure consistent input
    int seen[SIZE] = {0}; // array of seen using integer representation of bools

    for(int i = 0; i < SIZE; i++) {
        int current_num = sudoku[p -> row][i]; // get the current number going down the row by iterating that row's columns
        if(current_num > 9 || current_num < 1 || seen[current_num - 1]) return NULL; // if out of bounds or seen, leave results index as 0 and return null pointer
        seen[current_num - 1] = 1; // set number as seen
    }
    p -> results[p -> index] = 1; // set row as valid
    return NULL;
}

/* 
    Thread function that returns a null pointer and checks the given column for validity.
        - Sets an index of the results array to 1 corresponding to the index passed through the struct.
        - Leaves results array index as 0 if number is found to be out of bounds or repeated.
*/
void *check_column(void *arg) {
    parameters *p = (parameters *)arg; // copy of arg to ensure consistent input
    int seen[SIZE] = {0}; // array of seen numbers using integer representation of bools

    for(int i = 0; i < SIZE; i++) {
        int current_num = sudoku[i][p -> column]; // get the current number going down the column by iterating through it's rows
        if(current_num > 9 || current_num < 1 || seen[current_num - 1]) return NULL; // if out of bounds or seen, leave results index as 0 and return null pointer
        seen[current_num - 1] = 1; // set number as seen
    }
    p -> results[p -> index] = 1; // set column as valid
    return NULL;
}

/* 
    Thread function that returns a null pointer and checks the given grid for validity.
        - Sets an index of the results array to 1 corresponding to the index passed through the struct.
        - Leaves results array index as 0 if number is found to be out of bounds or repeated.
        - Defines a 3x3 grid using the column and row passed through the struct.
*/
void *check_grid(void *arg) { // sub grid
    parameters *p = (parameters *)arg; // copy of arg to ensure consistent input
    int seen[SIZE] = {0}; // array of seen using integer representation of bools

    for(int i = p -> row; i < p -> row + 3; i++) { // defines a row with the bound of 3 from the given row
        for(int j = p -> column; j < p -> column + 3; j++) { // defines a column with the bound of 3 from the given column
            int current_num = sudoku[i][j]; // get the current number in the grid
            if(current_num > 9 || current_num < 1 || seen[current_num - 1]) return NULL; // if out of bounds or seen, leave results index as 0 and return a null pointer
            seen[current_num - 1] = 1; // set number as seen
        }
    }
    p -> results[p -> index] = 1; // set grid as valid
    return NULL;
}

int main() {
    /* Standard legal Sudoku, 
     * you can use it to test your program

    int test_sudoku[9][9] = {
        {5, 3, 4, 6, 7, 8, 9, 1, 2},
        {6, 7, 2, 1, 9, 5, 3, 4, 8},
        {1, 9, 8, 3, 4, 2, 5, 6, 7},
        {8, 5, 9, 7, 6, 1, 4, 2, 3},
        {4, 2, 6, 8, 5, 3, 7, 9, 1},
        {7, 1, 3, 9, 2, 4, 8, 5, 6},
        {9, 6, 1, 5, 3, 7, 2, 8, 4},
        {2, 8, 7, 4, 1, 9, 6, 3, 5},
        {3, 4, 5, 2, 8, 6, 1, 7, 9}
    };
    */
    
    // reading from stdin
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            scanf("%d", &sudoku[i][j]); // enter column by column and when reach end, go down a row
        }
    }

    int results[27] = {0}; // an array that represents each row, column, and grid's validity with 0 being invalid default
    parameters *params[27]; // an array that holds the structs passed to the threads, needed to free malloc after joining
    pthread_t threads[27]; // 9 for each row, 9 for each column, 9 for each sub-grid

    // create row threads
    for(int i = 0; i < SIZE; i++) {
        parameters *data = (parameters *) malloc(sizeof(parameters)); // allocated dynamic memory on heap so the data remains consistent
        data -> row = i; // assigned as each row
        data -> column = 0; // stays at column 0
        data -> index = i; // index that corresponds to the results array
        data -> results = results; // pointer assigned to the results array pointer
        params[i] = data; // assigned this parameter to an array

        // create thread with entry point check_row with data as the argument with error catching for a return value of anything other than 0
        if (pthread_create(&threads[i], NULL, check_row, data) != 0) {
            free(params[i]); // free resource because we no longer need it
            perror("Failed to create thread"); // error message
            return EXIT_FAILURE;
        }
    }

    // create column threads
    for(int i = 0; i < SIZE; i++) {
        parameters *data = (parameters *) malloc(sizeof(parameters));
        data -> row = 0;
        data -> column = i;
        data -> index = 9 + i; // add 9 to the index as offset because we used up 9 for rows
        data -> results = results;
        params[9 + i] = data; // add 9 to the index as offset because we used up 9 for rows

        if (pthread_create(&threads[9 + i], NULL, check_column, data) != 0) {
            free(params[9 + i]); // 9 offset from rows
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    // create grid threads
    int column_displacement = 0;
    int row_displacement = 0;
    for(int i = 0; i < SIZE; i++) {
        parameters *data = (parameters *) malloc(sizeof(parameters));
        data -> row = row_displacement;
        data -> column = column_displacement;
        data -> index = 18 + i; // add 18 to the index as offset because we used up 9 for rows and 9 for columns
        data -> results = results;
        params[18 + i] = data; // add 18 to the index as offset because we used up 9 for rows and 9 for columns

        if((i + 1) % 3 == 0) { // if we reach the end, move down, otherwise, move to the right
            row_displacement += 3;
            column_displacement = 0;
        } else column_displacement += 3;

        if (pthread_create(&threads[18 + i], NULL, check_grid, data) != 0) {
            free(params[18 + i]); // 18 offset from rows and columns
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    // wait for all row threads to finish and free parameter structs used
    for(int i = 0; i < SIZE; i++) {
        if (pthread_join(threads[i], NULL) != 0) { // catch error and join thread
            free(params[i]);
            perror("Failed to join thread");
            return EXIT_FAILURE;
        } else free(params[i]);
    }

    // wait for all column threads to finish and free parameter structs used
    for(int i = 0; i < SIZE; i++) {
        if (pthread_join(threads[9 + i], NULL) != 0) {
            free(params[9 + i]);
            perror("Failed to join thread");
            return EXIT_FAILURE;
        } else free(params[9 + i]);
    }

    // wait for all grid threads to finish and free parameter structs used
    for(int i = 0; i < SIZE; i++) {
        if (pthread_join(threads[18 + i], NULL) != 0) {
            free(params[18 + i]);
            perror("Failed to join thread");
            return EXIT_FAILURE;
        } else free(params[18 + i]);
    }

    // print result, detection of a 0 means sudoku is invalid, otherwise, it is valid
    for(int i = 0; i < 27; i++) {
        if(results[i] == 0) {
            printf("0\n"); // means sudoku is invalid
            return 0;
        }
    }
    printf("1\n"); // means sudoku is valid

    return 0;
}
