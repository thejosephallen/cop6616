#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <omp.h>

/*
  int main(int argc, char* argv[])

  Heat flow solver.  All data is initially zero, boundary conditions are provided as input

  Inputs: 
  int N: size of the grid
  double leftBC: Dirichlet BC at left end
  double rightBC: Dirichlet BC at right end

  Outputs: Saves the final values.

*/

// Set the number of threads to be used
#define P 4

int main(int argc, char* argv[])
{
  // Get the number of grid points
  int N = atoi(argv[1]); 

  // Start the timer for measuring the time
  double starttime = omp_get_wtime();

  // Allocate memory for the grid
  double* u[2]; // Get the number of grid points
  u[0] = (double*)malloc(N * sizeof(double));  
  u[1] = (double*)malloc(N * sizeof(double));
	
  // Initialize the data
  int i, j;
#pragma omp parallel for private(i) shared(u,N) num_threads(P)
  for (i=0; i<N; ++i)
    u[0][i] = 0.;

  u[0][0] = atof(argv[2]);
  u[1][0] = u[0][0];

  u[0][N-1] = atof(argv[3]);
  u[1][N-1] = u[0][N-1];

  // CFL condition: dt < 0.5 dx^2
  double dx = 1./(N-1);
  double dx2 = dx*dx;
  double dt = 0.25 * dx2;

  // main loop: terminal time is T=1
#pragma omp parallel private(i) shared(u,dt,N,dx2) num_threads(P)
  for (i=0; i*dt < 1.0; ++i) {
#pragma omp parallel for private(j) 
    for (j=1; j<N-1; ++j) 
      u[(i+1)%2][j] = u[i%2][j]+dt*(u[i%2][j+1] - 2*u[i%2][j] + u[i%2][j-1])/dx2;
  }

  // store the final array in binary format to file
  FILE* fileid = fopen("finalu.dat", "w");
  fwrite(u[(i+1)%2], sizeof(double), N, fileid);
  fclose(fileid);

  // clean up
  free(u[0]);
  free(u[1]);

  // get the elapsed time
  double timeelapsed = omp_get_wtime()-starttime;
  printf("%d threads: Execution time = %le seconds.\n", P, timeelapsed);

  return 0;
}
