#!/bin/bash

export OMP_FIB=EDP

cd parboil/parboil/;

./parboil run histo omp_base large
