#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <mpi.h>

bool printout = false;  // Whether to print info about what is happening

// Utility functions
void invalid_args(int error);
int* get_rand_array(int size);
int **get_rand_square_matrix(int size);
int* flatten2Dmatrix(int **matrix, int size);
void print_array(int* array, int size);
void print_matrix(int **matrix, int size);
void swap(int *a, int *b);
int* merge(int* arrayParts, int size, int part);

// Option functions
int return_first(int* array);
int linear_search(int* array, int size, int key);
int* bubble_sort(int* array, int size);
int **matrix_multiply(int **matrix, int size);

int main(int argc, char *argv[]) {

	// Variable declarations
	int c;						// the number of array elements per process
	int N;						// length of array (N x N for matrices)
	int key;					// the random item in the array for linear search
	int option;					// specifies which function to execute in parallel
	int myrank;					// stores rank of current process
	int nprocs;					// number of processes (mpirun cmnd line arg)
	int* array;					// holds the array of random numbers
	int result;					// single per-process result for options 0 and 1q
	int **matrix;				// holds the random number square matrix
	int name_len;				// holds the len of the processor name
	int *arrayPart;				// a slice of the original array
	int* flat_matrix;			// flattened matrix for broadcasting
	int* result_array;			// holds process results for return first and linear search
	MPI_Status status;			// status variable for MPI
	double process_time;		// execution time for each process
	double elapsed_time;		// total time to execute a given option
	double process_time_sum;	// average execution time on parallelized work
	srand((unsigned)time(NULL));// Seeding time with null for RNG  

	// Initialize the Message Passing Interface
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	// Get the name of the current processor
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Get_processor_name(processor_name, &name_len);

	// Process the given arguments
	int error = -1;
	if (argc != 3) 
		error = 0;  // invalid arg count
	else {
		option = atoi(argv[1]);
		N = atoi(argv[2]);
	}
	if (N == 0 || N < nprocs || option > 3 || option < 0) 
		error = 1;  // invalid arg vals

	// If error, have master process print message, then finalize and exit
	if (error != -1){
		if (myrank == 0)
			invalid_args(error);
		MPI_Finalize();
		exit(0);
	}

	// Have the master process initialize the random data and whatever else is needed
	if (myrank == 0) {
		if (option != 3) {  // Generate a random array of size N
			array = get_rand_array(N);
			if (option == 1) // if search, get a random key from array
				key = array[rand() % N];
			if (option == 0 || option == 1) // allocate result array
				result_array = malloc(sizeof(int) * nprocs);			
			if (printout) {
				printf("Initial array:\n");
				print_array(array,N);
			}
		} else {  // Generate a random N x N square matrix
			matrix = get_rand_square_matrix(N);
			if (printout) {
				printf("Initial matrix:\n");
				print_matrix(matrix,N);
			}
		}
	}

	// Let processes self-identify
	// if (printout) {
	// 	int rank, size;
	// 	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	// 	MPI_Comm_size(MPI_COMM_WORLD, &size);
	// 	printf("\nI am %s: process %d out of %d",processor_name,rank+1,size);
	// }

	// Start timer and begin sending slices of data to each process
	elapsed_time -= MPI_Wtime();
	double start_time, final_time;
	c = ceil((float) N / nprocs); // data distribution isn't exact, has BUG if no processs gets a slice
	if (option != 3) {
		// scatter slices of array to each process
		arrayPart = malloc(sizeof(int) * c);
		MPI_Scatter(array, c, MPI_INT, arrayPart, c, MPI_INT, 0, MPI_COMM_WORLD);
	} else {
		if (myrank != 0)  // Initialize flat matrix for each non-master process
			flat_matrix = (int *)malloc(N*N*sizeof(int));
		else  // Have master process flatten the 2D matrix for transmission
			flat_matrix = flatten2Dmatrix(matrix, N);
		MPI_Bcast(flat_matrix, N * N, MPI_INT, 0, MPI_COMM_WORLD);
	}

	// Execute the desired option
	//if (printout && myrank == 0) printf("\nStarting the workers.");
	switch (option) {
		case 0:  // return first
			
			// Each process returns the first item in its slice of the array
			result = return_first(arrayPart);

			// Gather the results from each process
			MPI_Gather(&result, 1, MPI_INT, result_array, 1, MPI_INT, 0, MPI_COMM_WORLD);

			// Compute the execution time of each process
			process_time = elapsed_time + MPI_Wtime();
			MPI_Reduce(&process_time, &process_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if (myrank==0) printf("\nReturn first average process time = %f sec", process_time_sum / nprocs);


			// Have master process report the results			
			if (printout && myrank == 0) 
				printf("\nFirst item in the array is: %d",result_array[0]);

			break;
		case 1:  // linear search

			// Broadcast the search key to all processes
			MPI_Bcast(&key, 1, MPI_INT, 0, MPI_COMM_WORLD);

			// Each process searches an array slice for the key
			result = linear_search(arrayPart, c, key);

			// Gather the results from each process
			MPI_Gather(&result, 1, MPI_INT, result_array, 1, MPI_INT, 0, MPI_COMM_WORLD);

			// Compute the execution time of each process
			process_time = elapsed_time + MPI_Wtime();
			MPI_Reduce(&process_time, &process_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if (myrank==0) printf("\nSearch average process time = %f sec", process_time_sum / nprocs);

			// Have the master process report the results
			if (printout && myrank == 0) {				
				for (int i = 0; i < nprocs; i++) {
					//printf("\nProcess %d reported %d",i,result_array[i]);
					if (result_array[i] != -1)
						printf("\nKey %d found by process %d at index %d", key, i, (i * c) + result_array[i]);
				}
			}

			break;
		case 2:  // bubble sort
			
			// Have each process sort a slice
			arrayPart = bubble_sort(arrayPart, c);  // sorted sub-array

			// Gather results
			MPI_Gather(arrayPart, c, MPI_INT, array, c, MPI_INT, 0, MPI_COMM_WORLD);

			// Print sorta-sorted array
			if (printout && myrank == 0) {
				printf("\nSorta-sorted array:\n");
				print_array(array,N);
			}

			// Record average process time
			process_time = elapsed_time + MPI_Wtime();
			MPI_Reduce(&process_time, &process_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if (myrank == 0) printf("\nBubblesort avg time per process = %f sec \n", process_time_sum / nprocs);

			// Have master process merge the sorted sub-arrays from each process
			if (myrank == 0) {
				array = merge(array, N, c);
				//array = bubble_sort(array, N); // technically merging...
				if (printout) {
					printf("\nFinal sorted array:\n");
					print_array(array, N);
				}
			}
			break;
		case 3:  // matrix multiplication 

			// Have each process print its matrix slices
			if (printout) {
				for (int i = myrank * c * N, j = 0; i < (myrank * c * N) + (c * N) && i < N * N; i += N, j++) {
					printf("\nProcess %d gets row %d: ", myrank, (myrank * c) + j);
					print_array(&flat_matrix[i], N);
					printf("\n");
				}
			}

			// Allocate a flattened results matrix
		    int adj_rows = (myrank + 1) * c < N ? c : N - (myrank * c); // c unless last process has less rows than others
			int *results = calloc(adj_rows * N, sizeof(int)); // rows * cols * int size (CALLOC NOT MALLOC AHHH)

			// Perform matrix multiplication on the assigned rows of the flattened matrix (neat!)
			for (int i = myrank * c * N, j = 0; i < (myrank * c * N) + (adj_rows * N); i += N, j++)
			 	for (int k = 0; k < N; k++)
					for (int l = 0; l < N; l++)
			 			results[j * N + k] += flat_matrix[i + l] * flat_matrix[l * N + k];
			
			// Gather all relevant results
			MPI_Gather(results, adj_rows * N, MPI_INT, flat_matrix, adj_rows * N, MPI_INT, 0, MPI_COMM_WORLD);

			// Record average process time
			process_time = elapsed_time + MPI_Wtime();
			MPI_Reduce(&process_time, &process_time_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
			if (myrank == 0) printf("\nMatmul avg time per process = %f sec \n", process_time_sum / nprocs);

			// Report the final matrix
			if (printout && myrank == 0) {
				printf("\nFinal Matrix result:\n");
				for (int i = 0; i < N * N; i++) {
					if (i % N == 0) {
						print_array(&flat_matrix[i], N);
						printf("\n");
					}
				}
			}

			break;
	}

	// Report the final total execution time
	if (myrank == 0) 
		printf("\nExecution took: %f sec\n",elapsed_time += MPI_Wtime());

	MPI_Finalize();
	return 0;
}

void invalid_args(int error) {
	switch(error){
		case 0: printf("Error: Invalid number of arguments.\n"); break;
		case 1: printf("Error: Invalid argument values.\n"); break;
	}
	printf("Usage: <option> <data dimension>\n");
	printf("Valid options are:\n \
		0) Return First\n \
		1) Linear Search\n \
		2) Bubble Sort\n \
		3) Matrix Multiplication\n");
	printf("Valid data dimension must be greater than 0 and the number of processors.\n");
}

int* get_rand_array(int size) {
    srand((unsigned)time(NULL));    
    int* array = (int*) malloc(size * sizeof(int));
    for (int i = 0; i < size; i++)
        array[i] = rand() % 100000;
    return array;
}

int **get_rand_square_matrix(int size){
    srand((unsigned)time(NULL));
    int **matrix = malloc(sizeof(int*) * size);
    for(int i = 0; i < size; i++)
        matrix[i] = malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = rand() % 10000;
    return matrix;
}

int* flatten2Dmatrix(int **matrix, int size) {
	int *flat_matrix = (int *)malloc(size*size*sizeof(int));
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			flat_matrix[i * size + j] = matrix[i][j];
	return flat_matrix;
}

void print_array(int* array, int size) {
    for (int i = 0; i < size; i++)
        printf("%d ",*(array + i));
}

void print_matrix(int **matrix, int size) {
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++)
            printf("%d ",matrix[i][j]);
        printf("\n");
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int return_first(int* array) {
    return array[0];
}

int linear_search(int* array, int size, int key) {
	for(int i = 0; i < size; i++) {
		if(array[i] == key)
			return i;
	}
    return -1;
}

int* bubble_sort(int* array, int size) {
    for (int i = 0; i < size; i++) {
        int first = i % 2;
        for (int j = first; j < size - 1; j+=2)
            if (array[j] > array[j + 1])
                swap(&array[j],&array[j + 1]);
    }
	return array;
}

int **matrix_multiply(int **matrix, int size) {
    int **result = malloc(sizeof(int*) * size);
    for(int i = 0; i < size; i++)
        result[i] = malloc(sizeof(int*) * size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            result[i][j] = 0;
            for (int k = 0; k < size; k++)
                result[i][j] += matrix[i][k] * matrix[k][j];
        }
    }
    return result;
}

int* merge(int* arrayParts, int size, int part) {
	int* result = calloc(size, sizeof(int));

	int min_value = INT_MAX;
	int min_index;
	int removed = 0;
	while (removed < size) {

		// search for the min item at the front of each array part
		for (int i = 0; i < size; i += part) {
			if (arrayParts[i] != -1 && arrayParts[i] < min_value) {
				min_value = arrayParts[i];
				min_index = i;
			}
		}

		// shift everything else in the array left until part is reach
		int j; // could stop shifting once a -1 is reached...
		for (j = min_index; j < size && j < min_index + part - 1; j++)
			arrayParts[j] = arrayParts[j + 1];
		arrayParts[j] = -1; // set empties to -1

		// insert the min item to our new array
		result[removed++] = min_value;
		min_value = INT_MAX;
	}

	return result;
}
