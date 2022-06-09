#! /bin/bash
#SBATCH --exclusive
#SBATCH -p bench
#SBATCH -N 1

./cpy_test.out -s $((10*1024*1024))