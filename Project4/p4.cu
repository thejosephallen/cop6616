#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define NUM_THREADS 4
bool printout = false;

/********************** HOST FUNCTION DECLARATIONS ****************************/
void invalid_args(int error);
int* get_rand_array(int size);
int **get_rand_square_matrix(int size);
int* flatten2Dmatrix(int **matrix, int size);
void print_array(int* array, int size);
void print_matrix(int **matrix, int size);
int* merge(int* arrayParts, int size, int part);


/********************** DEVICE FUNCTIONS *************************************/

__global__ void return_first(int *array, int *result) {
   if (threadIdx.x == 0)
      atomicExch(result, array[0]);
   __syncthreads();
}

__global__ void linear_search(int *array, int *N, int *M, int *key, int *result) {
   int start = *M * threadIdx.x;
   int end = *N < start + *M ? *N : start + *M;   
   for (int i = start; i < end; i++)
      if (array[i] == *key)
         atomicExch(result, i);
   __syncthreads();
}

__global__ void bubble_sort(int *array, int *N, int *M) {
   int i, j, temp;
   int start = *M * threadIdx.x;
   int end = *N < start + *M ? *N : start + *M;   
   //printf("\nThread %d starts at index %d and ends at index %d", threadIdx.x, start, end);
   for (i = start; i < end; i++) {
      for (j = start; j < start+end-i-1; j++)
         if (array[j] > array[j + 1]) {
            temp = array[j];
            array[j] = array[j+1];
            array[j+1] = temp;
         }
   }
   __syncthreads();
}
__global__ void matrix_multiplication(int *d_flat_matrix, int *N, int *M, int *d_results) {
   int start = threadIdx.x * *M * *N;
   int end = *N * *N < start + *M * *N ? *N * *N : start + *M * *N;
   //printf("\nThread %d starts at index %d and ends at index %d", threadIdx.x, start, end);
   for (int i = start, j = 0; i < end; i += *N, j++)
      for (int k = 0; k < *N; k++)
         for (int l = 0; l < *N; l++)
            d_results[start + j * *N + k] += d_flat_matrix[i + l] * d_flat_matrix[l * *N + k];
   __syncthreads();
}

int main(int argc, char *argv[]) {

   // Declare host variables
   int option;       // the option/function to execute
   int N;            // the data dimension (N,1) for array & (N,N) for matrix
   int* array;       // the host's array of ints
   int** matrix;     // host copy of the square matrix
   int* flat_matrix; // the matrix, but flattened
   int size;         // the total amount of data
   int num_blocks;   // the number of CUDA blocks to use
   int num_threads = NUM_THREADS;  // the number of CUDA threads to use
   int M;            // the number of data items per thread
   int key;          // the key to find if doing linear search
   int result;       // single int result of return first and linear search

   // Declare device variables
   int* d_N;      // device copy of array size
   int* d_array;  // device copy of array
   int* d_M;      // device copy of elements per thread
   int* d_key;    // device copy of key if linear searching
   int* d_flat_matrix;// device copy of flattened 2D matrix
   int* d_result;  // device copy of single digit results
   int* d_results; // device copy of the result of matmul

   // Declare the CUDA timers
   cudaEvent_t start, stop;
   cudaEventCreate(&start);
   cudaEventCreate(&stop);

   // Process the given CLI arguments and process any errors
	int error = -1;
	if (argc != 3) 
		error = 0;  // invalid arg count
	else {
		option = atoi(argv[1]);
		N = atoi(argv[2]);
	}
	if (N == 0 || option > 3 || option < 0) 
		error = 1;  // invalid arg vals
	if (error != -1){
      invalid_args(error);
		exit(0);
	}

   // Initialize host variables
   size = N * sizeof(int);
   num_blocks = 1;
   M = ceil(1.0 * N / num_threads); // elements (or # rows) per thread

   // Allocate space on device for the host's option and data
   cudaMalloc((void **)&d_N, sizeof(int));
   cudaMalloc((void **)&d_M, sizeof(int));
   if (option == 3) { 
      cudaMalloc((void **)&d_flat_matrix, size * size);
      cudaMalloc((void **)&d_results, size * size);
      cudaMemset((void **)&d_results, 0, size * size);
   } else
      cudaMalloc((void **)&d_array, size);
   if (option == 0 || option == 1)
      cudaMalloc((void **)&d_result, sizeof(int));
   if (option == 1)
      cudaMalloc((void **)&d_key, sizeof(int));

   // Initialize the host's random data
   if (option == 3) {
      matrix = get_rand_square_matrix(N);
      if (printout) {
         printf("\nOriginal Matrix:\n");
         print_matrix(matrix, N);
      }
      flat_matrix = (int *) malloc(N * size);
      flat_matrix = flatten2Dmatrix(matrix, N);
   } else {
      array = get_rand_array(N);
      if (printout) {
         printf("\nOriginal Array:\n");
         print_array(array, N);
      }
      if (option == 1)
         key = array[rand() % N];
   }

   // Start timer to record total data transfer and function execution time
   cudaEventRecord(start, 0);

   // Copy host array, length, and elements per thread to device
   cudaMemcpy(d_N, &N, sizeof(int), cudaMemcpyHostToDevice);
   cudaMemcpy(d_M, &M, sizeof(int), cudaMemcpyHostToDevice);
   if (option == 3) {
      cudaMemcpy(d_flat_matrix, flat_matrix, size * size, cudaMemcpyHostToDevice);
   } else {
      cudaMemcpy(d_array, array, size, cudaMemcpyHostToDevice);
      if (option == 1)
         cudaMemcpy(d_key, &key, sizeof(int), cudaMemcpyHostToDevice);
   }

   // Execute option and copy result back to the host
   switch (option) {
      case 0:
         return_first<<<num_blocks, num_threads>>>(d_array, d_result);
         cudaMemcpy(&result, d_result, sizeof(int), cudaMemcpyDeviceToHost);
         break;
      case 1:
         linear_search<<<num_blocks, num_threads>>>(d_array, d_N, d_M, d_key, d_result);
         cudaMemcpy(&result, d_result, sizeof(int), cudaMemcpyDeviceToHost);
         break;
      case 2:
         bubble_sort<<<num_blocks, num_threads>>>(d_array, d_N, d_M);
         cudaMemcpy(array, d_array, size, cudaMemcpyDeviceToHost);
         break;
      case 3:
         matrix_multiplication<<<num_blocks, num_threads>>>(d_flat_matrix, d_N, d_M, d_results);
         cudaMemcpy(flat_matrix, d_results, size * size, cudaMemcpyDeviceToHost);
         break;
   }

   // Merge sorta-sorted array
   if (option == 2) {
      if (printout) {
         printf("\nSorta-sorted Array:\n");
         print_array(array, N);
      }   
      array = merge(array, N, M);
   }

   // end timer and report execution time
   cudaEventRecord(stop, 0);
   cudaEventSynchronize(stop);
   float elapsed_time;
   cudaEventElapsedTime(&elapsed_time, start, stop);
   cudaEventDestroy(start); cudaEventDestroy(stop);
   printf("\nExecution took %f ms", elapsed_time);

   // display results
   if (printout) {
      switch (option) {
         case 0:
            printf("\nThread 0 returned the first item in the array: %d", result);
            break;
         case 1:
            printf("\nSearch key %d found at index %d", key, result);
            break; // TODO calculate and report which thread found the key
         case 2:
            printf("\nSorted Array:\n");
            print_array(array, N);      
            break;
         case 3:
            printf("\nFinal Matrix result:\n");
            for (int i = 0; i < N * N; i++) {
               if (i % N == 0) {
                  print_array(&flat_matrix[i], N);
                  printf("\n");
               }
            }
            break;
      }
   }

   // Cleanup global CUDA variables
   if (option == 3)
      printf("\nError freeing memory on GPU (Result unaffected):\n");
   cudaFree(d_N); cudaFree(d_M);
   if (option == 3) {
      cudaFree(d_flat_matrix);
      cudaFree(d_results);
   } else {
      cudaFree(d_array);
      if (option == 1)
         cudaFree(d_key);
      if (option == 0 || option == 1)
         cudaFree(d_result);    
   }

   return 0;
}

/********************** HOST FUNCTIONS *************************************/
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
   int **matrix = (int **)malloc(sizeof(int*) * size);
   for(int i = 0; i < size; i++)
       matrix[i] = (int *)malloc(sizeof(int) * size);
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

int* merge(int* arrayParts, int size, int part) {
	int* result = (int*) calloc(size, sizeof(int));
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
		for (j = min_index; j < size - 1 && j < min_index + part - 1; j++)
			arrayParts[j] = arrayParts[j + 1];
		arrayParts[j] = -1; // set empties to -1
		// insert the min item to our new array
		result[removed++] = min_value;
		min_value = INT_MAX;
	}
	return result;
}
