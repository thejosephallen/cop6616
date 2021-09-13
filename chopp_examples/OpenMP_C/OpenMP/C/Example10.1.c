#include <stdio.h>
#include <stdlib.h>
#include <omp.h> // This is the header file for OpenMP directives

/*
  int main(int argc, char* argv[])

  The main program illustrates how to divide a loop into smaller
  pieces each handled in parallel by different threads.

  Inputs: argc should be 2
  argv[1] is the length of the array

  Outputs: Initializes an array in parallel

*/

int main(int argc, char* argv[]) 
{
  // Get the length of the array
  int N = atoi(argv[1]);

  // Allocate the memory for the array
  double* u = (double*) malloc(N*sizeof(double));

  int i;
  // Initialize an array of length N
#pragma omp parallel shared(u,N) private(i)
  {
#pragma omp for
    for (i=0; i<N; ++i) {
      printf("u[%d] is set by thread %d\n", i, omp_get_thread_num());
      u[i] = (double)i;
    }
  }

  return 0;
}
