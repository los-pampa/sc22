CC=/home/ipdps/gcc/build/bin/gcc #for reproducibility, change here accordingly.
FLAGS= -fopenmp -lm -O3
all: omp_ja

omp_ja:
	$(CC) omp_ja.c -o omp_ja $(FLAGS)

clean:
	rm omp_ja
