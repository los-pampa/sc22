CC=/path-to-gcc-bin/bin/gcc #for reproducibility, change here
FLAGS= -fopenmp -O3 -lm
all: fft_omp

fft_omp:
	$(CC) fft_omp.c -o fft_omp $(FLAGS)

clean:
	rm fft_omp
