CC=/home/ipdps/gcc/build/bin/gcc #change here accordingly
FLAGS= -fopenmp -lm -O3
all: omp_po

omp_po:
	$(CC) omp_po.c -o omp_po $(FLAGS)

clean:
	rm omp_po
