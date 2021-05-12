#!/bin/bash

N=(3 4 5 6 7 8 9 10 11 12 13 14)

echo -e "Average Values\n" > tsp_cuda_average_values.txt


AVG=(0 0 0 0 0 0 0 0 0 0 0 0)

for z in {0..11}
do
    for i in {0..4}
    do
        ./tsp_cuda  ${N[$z]}  > temp.txt

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

echo ${N[*]} >> tsp_cuda_average_values.txt
echo ${AVG[*]} >> tsp_cuda_average_values.txt

