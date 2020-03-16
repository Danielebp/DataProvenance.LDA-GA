#!/bin/bash

dataSize=("10" "20" "50" "100" "150" "200" "250")
idealCount=16

for size in ${dataSize[*]}; do
for it in {1..20};do
  ./main config.json -f 0.85 -dir ./data/data_${size}/ -out new_cuda2_${size}art_${it}it
  for log in ./data/data_${size}/new_cuda2_${size}art_${it}it/execution_*.log; do
    if [ -f "$log" ]
      then
        count=$(wc -l < ${log})
        if [ $count == $idealCount ]; then
           ./logParser -cpp -log ${log} -out ./simplerCudaParsedResults.txt
           mv ${log}  ${log}_
        fi
    fi
  done
done
done

