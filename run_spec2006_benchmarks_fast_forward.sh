#!/bin/sh

scons-3 -j 4 ./build/ARM/gem5.opt

echo "Out of Order execution with benchmarks branch predictor"
count=0
dir = $1
for bench in astar bzip2 hmmer namd lbm mcf sjeng;
do 
    echo -e "Executing bench $bench\n"
    ./build/ARM/gem5.opt -d "results/tage_scl/"$bench configs/spec2k6/run.py -b $bench\
    --maxinsts=1000000000 --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8\
    --fast-forward=1000000000 --warmup-insts=50000000 --standard-switch=50000000 --l1d_size=32kB --l1i_size=32kB --l2_size=2MB &
    count=$(( $count+1 ))

done

echo -e "Successfully executed $count total benchmarks"