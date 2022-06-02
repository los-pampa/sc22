#!/bin/bash

export OMP_FIB=EDP

./RODINIA/hotspot/hotspot 1024 1024 400000 20 RODINIA/data/hotspot/temp_1024 RODINIA/data/hotspot/power_1024 output.out

