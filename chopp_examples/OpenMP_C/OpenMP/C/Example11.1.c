#include <stdio.h>
#include <omp.h> 

/*
  int main(int argc, char* argv[])

  computes the number of positive numbers in each row and
  the total sum

  Inputs: none

  Outputs: Prints the counts for each row and the total sum
*/

int main(int argc, char* argv[]) 
{
  int M;
  int N;
  int a[4][16] = {10, 3,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		  6, 9, 10, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		  2, 3,  4, 5, 6, 8, 1, 3, 3, 3, 9, 3, 6, 8, 6, 9,
		  2, 9,  4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int sum = 0;
  int count[4] = {0, 0, 0, 0};

#pragma omp parallel num_threads(4) shared(N,M,a,count) reduction(+:sum)
  {
    // initialize the shared variables, only one thread needed
#pragma omp single
    {
      printf("Thread %d is initializing.\n", omp_get_thread_num());
      M = 4;
      N = 16;
    }

    // This code will be run by each thread in parallel
    int m = omp_get_thread_num();
    int i;
    for (i=0; i<N && a[m][i]!=0; ++i) {
      ++count[m];
      sum += a[m][i];
    }

    // use one thread to print the results
#pragma omp single
    {
      printf("Thread %d is reporting.\n", omp_get_thread_num());
      for (i=0; i<M; ++i)
	printf("Thread %d reports %d numbers.\n", i, count[i]);
    }
  }
	
  printf("The sum is %d\n", sum);
  return 0;
}
