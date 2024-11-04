#!/bin/bash

Параметры задач и процессов

tasks=(100)
processes=(20)



output_file="results4.txt"

num_threads=4



calculate_average() {
sum=0
for value in "$@"; do
sum=$(echo "$sum + $value" | bc)
done
average=$(echo "scale=4; $sum / ${#@}" | bc)
echo $average
}



for n in "${tasks[@]}"; do
for m in "${processes[@]}"; do
kach=()

    for ((i=0; i<100; i++)); do
        result=$(./a.out $num_threads $n $m)
        kach+=("$result")
        echo "Run: Threads=$num_threads, Tasks=$n, Processes=$m Run $((i + 1)) => Kach: $result"
    done
    average_time=$(calculate_average "${kach[@]}")

    echo "$n $m $average_time" >> $output_file
done

done