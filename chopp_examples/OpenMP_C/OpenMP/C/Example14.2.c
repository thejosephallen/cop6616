#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

/*
  int main(int argc, char* argv[])

  The main program generates a table of random integers

  Inputs: argc should be 3
  argv[1]: the length of the table
  argv[2]: the RNG seed

  Outputs: Computes the mean, max, and min of the data

*/

int main(int argc, char* argv[])
{
  int N = atoi(argv[1]); // input the length of each table
  long int seed = atol(argv[2]); // input seed value
  int i, k;
  int nums[omp_get_max_threads()][N]; // table of random variables

#pragma omp parallel shared(N, seed, nums) private(i, k)
  {
    // fill the table with random values
    srandom(seed);
    k = omp_get_thread_num();
    for (i=0; i<N; ++i)
      nums[k][i] = random()%100;
  }

  // print the results
  for (i=0; i<N; ++i) {
    for (k=0; k<omp_get_max_threads(); ++k)
      printf("%d\t", nums[k][i]);
    printf("\n");
  }

  return 0;
}
