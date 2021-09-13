#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 

/*
  int main(int argc, char* argv[])

  computes the number of instances of a given index

  Inputs: argc should be >= 2
  argv[*] input array of integers

  Outputs: Prints the sum of the integers 

*/

int main(int argc, char* argv[]) 
{
  // Convert the arguments into the input array
  int N = argc-1;
  int a[N];
  int i;
  for (i=0; i<N; ++i)
    a[i] = atoi(argv[i+1]);
  int sum = 0;

#pragma omp parallel num_threads(4) shared(N)
  {
#pragma omp for private(i) reduction(+:sum)
    for (i=0; i<N; ++i)
      sum += a[i];
  }
  printf("The sum is %d\n", sum);

  return 0;
}
