#!/bin/bash

export OMP_FIB=EDP
export OMP_PROC_BIND=close
export OMP_PLACES=cores

./JA/omp_ja

