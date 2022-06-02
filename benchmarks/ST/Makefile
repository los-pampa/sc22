CC=/home/ipdps/gcc/build/bin/gcc #for reproducibility, change here accordingly
FLAGS= -fopenmp -O3 -lm
all: stream

stream:
	$(CC) stream.c -o stream $(FLAGS)

clean:
	rm stream
