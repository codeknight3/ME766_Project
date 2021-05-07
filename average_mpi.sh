#!/bin/bash
AVG=(0 0 0 0 0 0 0 0 0 0 0 0)
N=(3 4 5 6 7 8 9 10 11 12 13 14)
Proc=(2 3 4 6 8 10 12)

echo -e "Average Values\n" > tsp_average_values.txt

for k in {0..6}
do
	for z in {0..11}
	do
		for i in {0..1}
		do
			mpirun -np ${Proc[$k]} ./out  ${N[$z]} > temp.txt

			while read time
			do
				echo $time
				AVG[$z]=$(echo "${AVG[$z]} + $time" | bc)
			done < temp.txt
		done 
	done

	for j in {0..11}
	do
		AVG[j]=$(echo "${AVG[j]} * 0.5" | bc)
	done

	echo ${Proc[$k]} >> tsp_average_values.txt
	echo ${N[*]} >> tsp_average_values.txt
	echo ${AVG[*]} >> tsp_average_values.txt

done 





