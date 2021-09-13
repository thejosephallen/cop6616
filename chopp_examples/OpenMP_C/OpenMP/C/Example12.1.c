#include <stdio.h>
#include <stdlib.h>
#include <omp.h> // This is the header file for OpenMP directives

/*
  int main(int argc, char* argv[])

  The main program illustrates how to divide threads into multiple
  distinct tasks

  Inputs: argc should be 2
  argv[1]: Length of the data set

  Outputs: Computes the mean, max, and min of the data

*/

int main(int argc, char* argv[]) 
{
  // Get the length of the array from the input arguments
  int N = atoi(argv[1]);

  int* u = (int*) malloc(N*sizeof(int));
  double mean;
  int maxu;
  int minu;
  int i;

  // Initialize the array with random integers
#pragma omp parallel private(i) shared(u) 
  {
#pragma omp for 
    for (i=0; i<N; ++i)
      u[i] = rand()%100;
  }
	
#pragma omp parallel private(i) shared(u,minu,maxu,mean) num_threads(4) 
  {
#pragma omp sections 
    {

      // This section will print the list of numbers
#pragma omp section 
      {
	printf("Thread %d will print the contents of the array\n", 
	       omp_get_thread_num());
	for (i=0; i<N-1; ++i)
	  printf("%d, ", u[i]);
	printf("%d\n", u[N-1]);
      }

      // This section computes the mean
#pragma omp section 
      {
	printf("Thread %d will compute the mean\n", 
	       omp_get_thread_num());
	mean = 0;
	for (i=0; i<N; ++i)
	  mean += u[i];
	mean /= (double)N;
      }	
			
      // This section computes the max
#pragma omp section 
      {
	printf("Thread %d will compute the max\n", 
	       omp_get_thread_num());
	maxu = u[0];
	for (i=1; i<N; ++i)
	  maxu = u[i] > maxu ? u[i] : maxu;
      }

      // This section computes the min
#pragma omp section 
      {
	printf("Thread %d will compute the min\n", 
	       omp_get_thread_num());
	minu = u[0];
	for (i=1; i<N; ++i)
	  minu = u[i] < minu ? u[i] : minu;
      }
    }
  }

  // print the results
  printf("mean(u) = %4.1f\n", mean);
  printf("max(u) = %d\n", maxu);
  printf("min(u) = %d\n", minu);

  return 0;
}
