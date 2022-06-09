#! /bin/bash

export OMP_NUM_THREADS=1

./cpy_test.out -s $((500*1024*1024))
echo "With " $OMP_NUM_THREADS "Threads"