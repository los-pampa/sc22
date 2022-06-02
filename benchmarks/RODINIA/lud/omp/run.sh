#!/bin/bash


export OMP_PLACES=SOCKETS
export OMP_PROC_BIND=CLOSE


for nt in 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 40
do
	export OMP_NUM_THREADS=$nt
	#for bench in bt.A.x cg.S.x ep.S.x ft.S.x is.S.x mg.W.x lu.S.x sp.A.x ua.S.x
	#for bench in cg.W.x is.W.x mg.A.x lu.W.x
	for bench in lud_omp
	do
		perf stat -e cpu-cycles,instructions,LLC-load-misses,LLC-store-misses,power/energy-pkg/,power/energy-ram/ -a -o OUT/perf.$bench.$nt.txt ./$bench -s 4096 >> OUT/nas.$bench.$nt.txt	
	done
done
