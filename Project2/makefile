parallel: parallel.o
	gcc -std=c99 -o parallel parallel.o -lgomp

parallel.o: parallel.c
	gcc -std=c99 -c parallel.c -fopenmp