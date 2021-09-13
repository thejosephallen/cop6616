#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h> 

/*
  int main(int argc, char* argv[])

  The main program illustrates how iterations are assigned to 
  threads according to the scheduling strategy

  Inputs: argc should be 3
  argv[1]: Number of iterations in the loop
  argv[2]: Block size used in the schedule clause

  Outputs: Time points when each iteration begins/ends
*/

int main(int argc, char* argv[]) 
{
  // Get the number of iterations and the block size from the arguments.
  int N = atoi(argv[1]);
  int bsize = atoi(argv[2]);

  // start the timer
  double t0 = omp_get_wtime();
  double t1, t2;
  int duration[N];

  int i;
  for (i=0; i<N; ++i) duration[i] = 1+rand()%5;
#pragma omp parallel shared(N) private(i,t1,t2) num_threads(4)
  {
#pragma omp for schedule(static,bsize)
    for (i=0; i<N; ++i)  {
      // report time of random sleep duration
      t1 = omp_get_wtime();
      sleep(duration[i]);
      t2 = omp_get_wtime();
      printf("Iteration %d: Thread %d, started %e, duration %e\n", 
	     i, omp_get_thread_num(), (double)(t1-t0), (double)(t2-t1));
    }
  }
  
  // stop the timer
  t2 = omp_get_wtime();
  printf("CPU time used: %.2f sec\n", t2-t0);
  return 0;
}
