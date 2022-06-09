#! /bin/bash

# export OMP_PLACES=cores
# export OMP_PROC_BIND=true
export OMP_SCHEDULE="static"
export OMP_NUM_THREADS=8

./matmul_test.out -n 10000 -m 10000
echo "With " $OMP_NUM_THREADS "Threads"