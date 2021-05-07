#!/bin/bash
AVG=(0 0 0 0 0 0 0 0 0 0 0 0 0)
N=(3 4 5 6 7 8 9 10 11 12 13 14 15)

for z in {0..12}
do
	for i in {0..1}
	do
		./tsp  ${N[$z]} > temp.txt

		while read time
		do
			echo $time
			AVG[$z]=$(echo "${AVG[$z]} + $time" | bc)
		done < temp.txt
	done 
done 

for i in {0..12}
do
	AVG[i]=$(echo "${AVG[i]} * 0.5" | bc)
done

echo -e "Average Values\n" > tsp_average_values.txt
echo ${N[*]} >> tsp_average_values.txt
echo ${AVG[*]} >> tsp_average_values.txt
