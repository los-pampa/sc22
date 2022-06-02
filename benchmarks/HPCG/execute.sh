#!/bin/bash

export OMP_FIB=EDP
export OMP_PROC_BIND=close
export OMP_PLACES=cores

./HPCG/HPCCG_BIN 256 256 128

