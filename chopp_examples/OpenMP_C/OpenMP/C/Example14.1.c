#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <fftw3.h>

/*
  int main(int argc, char* argv[])

  The main program illustrates how to use the multi-threaded
  version of the FFTW library

  Inputs: none

  Outputs: Prints the original data and the result of a forward
  and backward transform for comparison

*/

int main(int argc, char* argv[])
{
#ifndef M_PI
  const double M_PI = 4.0*atan(1.0);
#endif
  int N = 16;
  fftw_complex *in, *out, *out2;
  fftw_plan p, pinv;
	
  // initialize the threads to be used in the FFT execution
  fftw_init_threads();
	
  // allocate memory for the data
  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
  out2 = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
	
  // create an FFT plan that uses all the available threads
  fftw_plan_with_nthreads(omp_get_max_threads());
	
  // construct the plans for the forward and backward transforms
  p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  pinv = fftw_plan_dft_1d(N, out, out2, FFTW_BACKWARD, FFTW_ESTIMATE);
	
  // construct the initial data
  for (int i=0; i<N; ++i) {
    double dx = 2.*M_PI/N;
    in[i][0] = sin(i*dx);
    in[i][1] = 0.;
  }
	
  // perform the forward transform, results are in variable out
  fftw_execute(p);
	
  // perform the backward transform, results are in out2
  fftw_execute(pinv);
	
  // print the original data and the results for comparison
  for (int i=0; i<N; ++i)
    printf("a[%d]: %f + %fi\n", (i<=N/2 ? i : i-N), out[i][0]/N, 
	   out[i][1]/N);
  printf("---\n");
  for (int i=0; i<N; ++i)
    printf("f[%d]: %f + %fi == %f + %fi\n", i, out2[i][0]/N, 
	   out2[i][1]/N, in[i][0], in[i][1]);

  // clean up the threads from OpenMP
  fftw_cleanup_threads();
	
  // free the remaining memory
  fftw_destroy_plan(p);
  fftw_destroy_plan(pinv);
  fftw_free(in);
  fftw_free(out);
  fftw_free(out2);
}
