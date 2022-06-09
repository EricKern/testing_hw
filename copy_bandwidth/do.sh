#! /bin/bash

# export OMP_PLACES=cores
# export OMP_PROC_BIND=true
export OMP_NUM_THREADS=8

./cpy_test.out -s $((500*1024*1024))
echo "With " $OMP_NUM_THREADS "Threads"