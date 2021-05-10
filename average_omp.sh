#!/bin/bash
#SBATCH -N 1
#SBATCH --ntasks-per-node=20
#SBATCH --time=04:00:00
#SBATCH --job-name=omp_timing
#SBATCH --error=job.%J.err
#SBATCH --output=job.%J.out
#SBATCH --partition=standard

ulimit -s unlimited

module load compiler/gcc

N=(3 4 5 6 7 8 9 10 11 12 13)
Thrd=(2 3 4 6 8 12 16 20)
echo -e "Average Values\n" > tsp_average_values.txt
for k in {0..7}
do
	AVG=(0 0 0 0 0 0 0 0 0 0 0)
	for z in {0..10}
	do
		for i in {0..1}
		do
			./tsp  ${N[$z]} ${Thrd[$k]}  > temp.txt
			while read time
			do
				echo $time
				AVG[$z]=$(echo "${AVG[$z]} + $time" | bc)
			done < temp.txt
		done
	done
	for j in {0..10}
	do
		AVG[j]=$(echo "${AVG[j]} * 0.5" | bc)
	done

	echo ${Thrd[$k]} >> tsp_average_values.txt
	echo ${N[*]} >> tsp_average_values.txt
	echo ${AVG[*]} >> tsp_average_values.txt
done
