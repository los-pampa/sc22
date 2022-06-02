#!/bin/bash

export OMP_FIB=EDP

./RODINIA/streamcluster/sc_omp 10 20 5000 65536 65536 1000 none output.txt 4 

