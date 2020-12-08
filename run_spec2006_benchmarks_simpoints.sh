#!/bin/sh

#scons-3 -j 4 ./build/ARM/gem5.opt

echo "Out of Order execution with WormHole_benchmarks branch predictor"
count=0
i=0
for bench in astar bzip2 hmmer lbm mcf namd sjeng;
do 
    echo -e "Executing bench $bench\n"
    ./build/ARM/gem5.opt -d wise_pred/simpoint/$bench configs/spec2k6/run.py -b $bench --simpoint-profile --simpoint-interval 10000000 --cpu-type=NonCachingSimpleCPU &
    count=$(( $count+1 ))

done

echo -e "Successfully executed $count total benchmarks"


# astar bwaves bzip2 cactusADM calculix GemsFDTD gobmk h264ref hmmer lbm leslie3d libquantum mcf milc namd omnetpp povray sjeng sphinx3 xalancbmk

# /build/ARM/gem5.opt -d ../Project/TAGE_WISE/$bench configs/spec2k6/run.py -b $bench\
#     --maxinsts=200000000 --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8\
#     --fast-forward=1000000000 --warmup-insts=50000000 --standard-switch=50000000 --l1d_size=32kB --l1i_size=32kB --l2_size=2MB

# ./build/ARM/gem5.opt -d wise/hmmer configs/spec2k6/run.py -b hmmer\
#     --maxinsts=2000000 --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8\
#     --l1d_size=32kB --l1i_size=32kB --l2_size=2MB


# ./build/ARM/gem5.opt -d ../Project/TAGE_WISE_smaller/$bench configs/spec2k6/run.py -b $bench\
#     --maxinsts=20000000 --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8\
#     --fast-forward=1000000000 --warmup-insts=50000000 --standard-switch=50000000 --l1d_size=32kB --l1i_size=32kB --l2_size=2MB
