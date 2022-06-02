CC=/home/ipdps/gcc/build/bin/gcc # for reproducibility, change it acordingly
FLAGS= -fopenmp -O3 -lm
all: omp_nbody
omp_nbody:
	$(CC) omp_nbody.c -o omp_nbody $(FLAGS)

clean:
	rm omp_nbody
