#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 

/*
  int main(int argc, char* argv[])

  computes the number of instances of a given index

  Inputs: input array of integers

  Outputs: Prints the count of integers that match the thread ID

*/

int main(int argc, char* argv[]) 
{
  // number of arguments is the length of the list except for argv[0]
  int N = argc-1; 
  int a[N];
  int i;
  // convert the arguments into an array of ints
  for (i=0; i<N; ++i)
    a[i] = atoi(argv[i+1]);

#pragma omp parallel num_threads(4) shared(N) 
  {
    int j;
    int count = 0;
    // each thread counts how many inputs match their thread id
    for (j=0; j<N; ++j) 
      count += a[j] == omp_get_thread_num() ? 1 : 0;
    printf("The number %d appears in the list %d time(s).\n", 
	   omp_get_thread_num(), count);
  }

  return 0;
}
