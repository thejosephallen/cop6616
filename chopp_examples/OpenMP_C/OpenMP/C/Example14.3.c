#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

/*
  int main(int argc, char* argv[])

  The main program generates a table of random integers

  Inputs: argc should be 3
  argv[1]: Length of the table
  argv[2]: Initial RNG seed

  Outputs: Computes the mean, max, and min of the data

*/

int main(int argc, char* argv[])
{
  int N = atoi(argv[1]); // input the length of each table
  long int seed = atol(argv[2]); // input seed value
  int i, k;
  int nums[omp_get_max_threads()][N]; // table of random variables
  struct random_data* state; // pointer to random number state 

#pragma omp parallel shared(N, seed, nums) private(i, k, state) 
  {
    k = omp_get_thread_num();
		
    // allocate space for the state variable 
    state = malloc(sizeof(struct random_data)); 
		
    // populate the state variable for each thread
    char statebuf[32]; 
    initstate_r(seed, statebuf, 32, state); 
		
    // set the initial seed with thread-safe version
    srandom_r(seed, state);
    int temp;
    for (i=0; i<N; ++i) {
      // get a random value using the thread-safe version
      random_r(state, &temp);
      nums[k][i] = temp%100;
    }
			
    // release the memory allocated for the state
    free(state);
  }

  // print the results
  for (i=0; i<N; ++i) {
    for (k=0; k<omp_get_max_threads(); ++k)
      printf("%d\t", nums[k][i]);
    printf("\n");
  }

  return 0;
}
