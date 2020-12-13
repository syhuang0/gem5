#!/bin/sh

# scons-3 -j 4 ./build/ARM/gem5.opt

echo "Out of Order execution with benchmarks branch predictor"
count=0
# dir = $1
total=12

spec2k6=(calculix h264ref omnetpp povray)
i=0
while [ $i -lt ${#spec2k6[@]} ]
do 
    proc=$(ps aux | grep $USER | grep "tage" | grep -v "grep" | wc -l)
    bench=${spec2k6[$i]}
    if [ $proc -lt $total ]
    then
        
        echo -e "Executing bench ${spec2k6[$i]}"
    ./build/X86/gem5.opt -d "results/tage_scl/x86/"$bench configs/spec2k6/run.py -b $bench\
    --maxinsts=1000000000 --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8\
    --fast-forward=1000000000 --warmup-insts=50000000 --standard-switch=50000000 --l1d_size=32kB --l1i_size=32kB --l2_size=2MB &
        i=$(( $i+1 ))
    else
        # echo -e "Waiting to execute $bench"
        # echo -e "i: $i"
        sleep 30m
    fi
    # echo -e "Executing bench $bench\n"
    # ./build/X86/gem5.opt -d "results/wisl_tage/x86/"$bench configs/spec2k6/run.py -b $bench\
    # --maxinsts=1000000000 --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8\
    # --fast-forward=1000000000 --warmup-insts=50000000 --standard-switch=50000000 --l1d_size=32kB --l1i_size=32kB --l2_size=2MB &
    # count=$(( $count+1 ))

done

# echo -e "Successfully executed $count total benchmarks"

# do next
#  calculix h264ref omnetpp povray sphinx3 
# astar bzip2 hmmer namd lbm mcf sjeng
