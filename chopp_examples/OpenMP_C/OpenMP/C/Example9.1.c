#include <stdio.h>
#include <omp.h> // This is the header file for OpenMP directives

/*
  int main(int argc, char* argv[])

  The main program is the starting point for an OpenMP program.
  This one simply reports its own thread number.

  Inputs: none

  Outputs: Prints "Hello World #" where # is the thread number.

*/

int main(int argc, char* argv[]) 
{

  // OpenMP does parallelism through the use of threads.
  // #pragma are compiler directives that are handled by OpenMP
  // The directive applies to the next simple or compound statement
  // By default, the number of threads created will correspond to 
  // the number of cores in the computer.
#pragma omp parallel

  // This is the simple statement that will be done in parallel threads
  printf("Hello World! I am thread %d out of %d\n", 
	 omp_get_thread_num(), omp_get_num_threads());
  printf("All done with the parallel code.\n");

  return 0;
}
