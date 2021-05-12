#!/bin/bash

N=(3 4 5 6 7 8 9 10 11 12 13 14)
Thrd=(2 3 4 6 8 12 16 20)

echo -e "Average Values\n" > tsp_omp_average_values.txt

for k in {0..7}
do
	AVG=(0 0 0 0 0 0 0 0 0 0 0 0)

	for z in {0..11}
	do
		for i in {0..4}
		do
			./tsp_omp  ${N[$z]} ${Thrd[$k]}  > temp.txt

			while read time
			do
				echo $time
				AVG[$z]=$(echo "${AVG[$z]} + $time" | bc)
			done < temp.txt
		done
	done

	for j in {0..11}
	do
		AVG[j]=$(echo "${AVG[j]} * 0.2" | bc)
	done

	echo ${Thrd[$k]} >> tsp_omp_average_values.txt
	echo ${N[*]} >> tsp_omp_average_values.txt
	echo ${AVG[*]} >> tsp_omp_average_values.txt
done
