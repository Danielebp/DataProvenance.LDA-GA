#!/bin/bash

#dataSize=("10" "20" "50" "100" "150" "200" "250")
dataSize=("20")
#libraries=("wlda" "glda" "blda")
libraries=("glda")
idealCount=17

for size in ${dataSize[*]}; do
for it in {1..6};do
for lib in ${libraries[*]}; do
  ./main config.json -cpu -f 0.85 -dir ./data/_data_${size}/ -${lib} -out test_${lib}_${size}art_${it}it
  for log in ./data/_data_${size}/test_${lib}_${size}art_${it}it/execution_*.log; do
    if [ -f "$log" ]
      then
        count=$(wc -l < ${log})
        if [ $count == $idealCount ]; then
           ./logParser -cpp -log ${log} -out ./combinedParsedResults_20.txt
           mv ${log}  ${log}_
        fi
    fi
  done
done
done
done

