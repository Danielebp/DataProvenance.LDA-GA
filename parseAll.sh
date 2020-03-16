#!/bin/bash

dataSize=("10" "20" "50" "100" "150" "200" "250")
idealCount=16

for size in ${dataSize[*]}; do
for fit in {10..95..10}; do
for it in {1..10};do
for log in ./data/data_${size}/new_cpp_${size}art_${fit}fit_${it}it/execution_*.log; do
  if [ -f "$log" ]
    then
        count=$(wc -l < ${log})
        if [ $count == $idealCount ]; then
	   ./logParser -cpp -log ${log} -out ./parsedResults.txt
	   mv ${log}  ${log}_
        fi
  fi
done
done
done
done

tail ./parsedResults.txt

