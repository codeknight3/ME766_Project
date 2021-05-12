#!/bin/bash

N=(12 13 14)
Proc=(2 4 5 10)
Thrd=(2 4 5 10)

echo -e "Average Values\n" > tsp_mpi_average_values.txt

for k in {0..3}
do
	for l in {0..3}
	do
		AVG=(0 0 0)

		for z in {0..2}
		do
			for i in {0..4}
			do
				mpirun -np ${Proc[$k]} ./tsp_hybrid ${N[$z]} ${Thrd[$l]} > temp.txt

				while read time
				do
					echo $time
					AVG[$z]=$(echo "${AVG[$z]} + $time" | bc)
				done < temp.txt
			done 
		done

		for j in {0..2}
		do
			AVG[j]=$(echo "${AVG[j]} * 0.2" | bc)
		done

		echo ${Proc[$k]} >> tsp_mpi_average_values.txt
		echo ${N[*]} >> tsp_mpi_average_values.txt
		echo ${AVG[*]} >> tsp_mpi_average_values.txt

	done
done 