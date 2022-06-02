#!/bin/bash

export OMP_FIB=EDP
export OMP_PROC_BIND=close
export OMP_PLACES=cores

cd parboil/parboil/;
./parboil run mri-gridding omp_base small
