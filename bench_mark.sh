#!/bin/sh


write_thresh()
{
	echo "#define BDGTHRESH $1" >src/cpu/minor/bgdthresh.hh
}


run_bench()
{
	nice -n 10 ./build/ARM/gem5.opt -d "$2/$1" configs/spec2k6/run.py -b "$1" --cpu-type=DerivO3CPU --caches --l2cache --l1d_assoc=2 --l1i_assoc=2 --l2_assoc=8 --l1d_size=32kB --l1i_size=32kB --l2_size=2MB --cpu-clock=1000000000 --warmup-insts=500000 --standard-switch=500000 --maxinsts=1000000000
	
}


branch_name="$1"

shift

bench_list="$@"

for j in 0;
do
	#echo "BGDTHRESH=$j"
	#write_thresh "$j"
        #scons-3 -j 8 ./build/ARM/gem5.opt
	sleep 1
	for i in $bench_list;
	do
    		echo "start $i"
    		run_bench "$i" "$branch_name" &
                process_id="process_id $!"
	done
        for i in $process_id;
	do
		wait "$i"
	done
done
