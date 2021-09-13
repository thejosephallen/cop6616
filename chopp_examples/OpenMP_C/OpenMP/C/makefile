EXEC= Example9.1  Example9.2  Example10.1 Example10.2 Example10.3 Example10.4 \
      Example10.5 Example11.1 Example11.2 Example11.3 Example12.1 Example13.1 \
      Example13.2 Example14.1 Example14.2 Example14.3 Example14.4

all: $(EXEC)

Example9.1: Example9.1.c
	gcc -o Example9.1 Example9.1.c -fopenmp -lgomp

Example9.2: Example9.2.c
	gcc -o Example9.2 Example9.2.c -fopenmp -lgomp

Example10.1: Example10.1.c
	gcc -o Example10.1 Example10.1.c -fopenmp -lgomp

Example10.2: Example10.2.c
	gcc -o Example10.2 Example10.2.c -fopenmp -lgomp

Example10.3: Example10.3.c
	gcc -o Example10.3 Example10.3.c -fopenmp -lgomp

Example10.4: Example10.4.c
	gcc -o Example10.4 Example10.4.c -fopenmp -lgomp

Example10.5: Example10.5.c
	gcc -o Example10.5 Example10.5.c -fopenmp -lgomp

Example11.1: Example11.1.c
	gcc -o Example11.1 Example11.1.c -fopenmp -lgomp

Example11.2: Example11.2.c
	gcc -o Example11.2 Example11.2.c -fopenmp -lgomp

Example11.3: Example11.3.c
	gcc -o Example11.3 Example11.3.c -fopenmp -lgomp

Example12.1: Example12.1.c
	gcc -o Example12.1 Example12.1.c -fopenmp -lgomp

Example13.1: Example13.1.c
	gcc -o Example13.1 Example13.1.c -fopenmp -lgomp

Example13.2: Example13.2.c
	gcc -o Example13.2 Example13.2.c -fopenmp -lgomp

Example14.1: Example14.1.c
	gcc -o Example14.1 Example14.1.c -fopenmp -lgomp -lfftw3_omp -lfftw3 -lm

Example14.2: Example14.2.c
	gcc -o Example14.2 Example14.2.c -fopenmp -lgomp

Example14.3: Example14.3.c
	gcc -o Example14.3 Example14.3.c -fopenmp -lgomp

Example14.4: Example14.4.c
	gcc -o Example14.4 Example14.4.c -fopenmp -lgomp


clean: 
	-rm $(EXEC)
	-rm *~
	-rm *.dat
