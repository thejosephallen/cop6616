#include <stdio.h>
#include <stdlib.h> // RAND_MAX is defined in here
#include <omp.h> 

/*
  int main(int argc, char* argv[])

  computes the number of values less than RAND_MAX/2
  in a random array

  Inputs: argc should be 2
  argv[1]: length of the random array

  Outputs: Prints the number of values less than RAND_MAX/2
*/

int main(int argc, char* argv[]) 
{
  // Get the number of random values to sample from the arg list
  int N = atoi(argv[1]);
  int a[N];
	
  // Use mid as the cutoff value
  int mid = RAND_MAX/2;
  int i;
  int count = 0;

  // Generate the random values
#pragma omp parallel for private(i) shared(a,N)
  for(i=0; i<N; ++i)
    a[i] = rand();

#pragma omp parallel private(i) shared(a,N,mid,count)
  {
    // initialize the shared variable count using only one thread
#pragma omp single
    {
      count = 0;
    }

#pragma omp for 
    for (i=0; i<N; ++i) {
      if (a[i] < mid) {
	// make sure count is updated one thread at a time
#pragma omp atomic
	++count;
      }
    }
  }
	
  printf("%d/%d numbers are below %d\n", count, N, mid);
  return 0;
}
