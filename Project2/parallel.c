#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <stdbool.h>

int NUM_THREADS = 8;
bool printout = false;

// Utilities
int* get_rand_array(int size);
int **get_rand_square_matrix(int size);
void print_array(int* array, int size);
void print_matrix(int **matrix, int size);
//void del_array(int* array,int size);
//void del_matrix(int **matrix, int size);

// Parallel Algorithms
int return_first(int* array);
int linear_search(int* array, int size, int key);
void bubble_sort(int* array, int size);
void quicksort_wrapper(int* array, int size);
void quicksort(int* array, int low, int high);
int partition(int* array, int low, int high);
int **matrix_multiply(int **matrix, int size);

void swap(int* a, int* b);

int main(int argc, char *argv[]){

    // verify program input
    if (argc != 3){
        printf("Usage: <option> <array/matrix size>");
        exit(0);
    }
    int option = atoi(argv[1]);
    int N = atoi(argv[2]);
    if (N == 0 || option > 3 || option < 0){
        printf("\nPlease enter better arguments.");
        printf("\nOptions: 0)Return First, 1)Linear Search, 2)Bubble Sort, 3) Matrix Multiplication\n");
        exit(0);
    }

    // Switch to execute the right code
    int* array;
    switch(option){

        case 0: // Return First
            
            printf("\nGenerating Array...");
            //int *array = get_rand_array(N);
            array = get_rand_array(N);
            if (printout) printf("\nOriginal Array:\n");
            if (printout) print_array(array,N);

            printf("Returning first...");
            int first = return_first(array);
            break;

        case 1: // Linear Search

            printf("\nGenerating Array...");
            array = get_rand_array(N);
            if (printout) printf("\nOriginal Array:\n");
            if (printout) print_array(array,N);

            int rand_key = array[rand() % N];
            printf("\nSearching for randomly-selected key: %d...",rand_key);
            int result = linear_search(array, N, rand_key);
            break;

        case 2:  // Bubble Sort

            printf("\nGenerating Array...");
            array = get_rand_array(N);
            if (printout) printf("\nOriginal Array:\n");
            if (printout) print_array(array,N);

            printf("\nBubble sorting the array...");
            bubble_sort(array,N);
            if (printout) printf("\nSorted Array:\n");
            if (printout) print_array(array,N);
            break;

        case 3: // Matrix Multiplication
            printf("\nGenerating Matrix...");
            int **matrix = get_rand_square_matrix(N);
            if (printout) printf("\nOriginal Matrix:\n");
            if (printout) print_matrix(matrix,N); 

            printf("\nMultiplying Matrix...");
            int **mmatrix = matrix_multiply(matrix, N);
            if (printout) printf("\nMultiplied Matrix:\n");
            if (printout) print_matrix(matrix,N);
            break;

        default:
            //quicksort_wrapper(array, N);
            break;
    }

    printf("\n");
    return 0;

}

/*=============================================================================
A function to generate a random array of certain size.
=============================================================================*/
int* get_rand_array(int size){
    srand((unsigned)time(NULL));    
    int* array = (int*) malloc(size * sizeof(int));
    for (int i = 0; i < size; i++){
        array[i] = rand() % 100000;
    }
    return array;
}

/*=============================================================================
A function to generate a square matrix of random numbers.
=============================================================================*/
int **get_rand_square_matrix(int size){
    srand((unsigned)time(NULL));
    int **matrix = malloc(sizeof(int*) * size);
    for(int i = 0; i < size; i++){
        matrix[i] = malloc(sizeof(int*) * size);
    }
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++){
            matrix[i][j] = rand() % 10000;
        }
    }
    return matrix;
}

/*=============================================================================
A "parallel" algorithm to return the first item in an array. 
=============================================================================*/
int return_first(int* array){
    clock_t begin = clock();
    printf("\nFirst: %d",array[0]);
    double elapsed = ((double)(clock() - begin)) / CLOCKS_PER_SEC;
    printf("\nExecution took %f seconds\n",elapsed);
    return array[0];
}

/*=============================================================================
A parallel implementation of linear search using openmp.
=============================================================================*/
int linear_search(int* array, int size, int key){
    double start_time, elapsed_time;
    int i;
    start_time = omp_get_wtime();
    #pragma omp parallel num_threads(NUM_THREADS) private(i)
    {
        #pragma omp for
        for(i = 0; i < size; i++){
            if(array[i] == key){
                printf("\nKey found. Position = %d by thread %d \n",     i+1, omp_get_thread_num());
            }
        }
    }
    elapsed_time = omp_get_wtime() - start_time;
    printf("\nExecution completed in %f seconds",elapsed_time);
    return 0;
}
/*=============================================================================
A bubble sort implemented using even-odd sorting methods.
=============================================================================*/
void bubble_sort(int* array, int size){
    double start_time, elapsed_time;
    int tid;
    start_time = omp_get_wtime();
    for (int i = 0; i < size; i++){
        int first = i % 2;
        #pragma omp parallel for default(none), shared(array,first,size) num_threads(NUM_THREADS)
        for (int j = first; j < size - 1; j+=2){
            if (array[j] > array[j + 1]){
                swap(&array[j],&array[j + 1]);
            }
        }
    }
    elapsed_time = omp_get_wtime() - start_time;
    printf("\nExecution completed in %f seconds",elapsed_time);
}

/*=============================================================================
A parallel implementation of quicksort using openmp.
=============================================================================*/
void quicksort_wrapper(int* array, int size){

    // printf("\nUnsorted array:\n");
    // print_array(array,size);

    double start_time, elapsed_time;
    start_time = omp_get_wtime();

    quicksort(array, 0 , size - 1);

    elapsed_time = omp_get_wtime() - start_time;
    printf("\nWork completed in %f seconds",elapsed_time);

    // printf("\nSorted Array:\n");
    // print_array(array,size);
}

void quicksort(int* array, int low, int high){

    if (low < high){

        // partition with array[low] as pivot
        int s = low; // left of s is lower than pivot
        for (int i = s + 1; i <= high; i++){
            if (array[i] <= array[low])
                swap(&array[++s],&array[i]);
        }
        swap(&array[low],&array[s]);

        // maybe use tasks?:
            // https://codereview.stackexchange.com/questions/181441/making-a-faster-parallel-quicksort
            // https://github.com/eduardlopez/quicksort-parallel/blob/master/quicksort-omp.h
        // recurse on each half
        #pragma omp parallel sections num_threads(NUM_THREADS) 
        {
            #pragma omp section
            quicksort(array, low, s - 1);
        
            #pragma omp section 
            quicksort(array, s + 1, high);
        }
    }
}

void swap(int *a, int *b){
    int temp = *a;
    *a = *b;
    *b = temp;
}

/*=============================================================================
A parallel implementation of matrix multiplication of a square matrix with
itself.
=============================================================================*/
int **matrix_multiply(int **matrix, int size){
    double start_time, elapsed_time;
    int i, j, k;
    int **result = malloc(sizeof(int*) * size);
    for(int i = 0; i < size; i++){
        result[i] = malloc(sizeof(int*) * size);
    }
    start_time = omp_get_wtime();
    # pragma omp parallel shared (result, matrix) \
    private ( i, j, k) num_threads(NUM_THREADS)
    {
        # pragma omp for
        for (i = 0; i < size; i++){
            for (j = 0; j < size; j++){
                result[i][j] = 0;
                for (k = 0; k < size; k++){
                    result[i][j] += matrix[i][k] * matrix[k][j];
                }
            }
        }
    }
    elapsed_time = omp_get_wtime() - start_time;
    printf("\nExecution completed in %f seconds",elapsed_time);
    return result;
}

/*=============================================================================
A function to print an array.
=============================================================================*/
void print_array(int* array, int size){
    for (int i = 0; i < size; i++)
        printf("%d ",*(array + i));
}

/*=============================================================================
A function to print a matrix.
=============================================================================*/
void print_matrix(int **matrix, int size){
    for (int i = 0; i < size; i++){
        for (int j = 0; j < size; j++)
            printf("%d ",matrix[i][j]);
        printf("\n");
    }
}
