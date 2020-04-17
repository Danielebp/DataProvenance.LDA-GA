#!/bin/bash

dataSize=("10" "50" "100" "150" "200" "300" "400" "500")
libraries=("wlda" "glda" "blda" "gibbs")
idealCount=17
timestamp=$(date "+%Y%m%d%H%M%S")
combinedFile=./combinedParsedResults_$timestamp.txt

for size in ${dataSize[*]}; do
for version in {1..10}; do
for it in {1..5};do
for lib in ${libraries[*]}; do
  ./provenance config.json -f 0.85 -dir ./new_data/data_${size}/ -${lib} -out test_${lib}_${timestamp}_${version}_${it} -truth input_${version}.truth -prep input_${version}.txt
  for log in ./new_data/data_${size}/test_${lib}_${timestamp}_${version}_${it}/execution_*.log; do
    if [ -f "$log" ]
      then
        count=$(wc -l < ${log})
        if [ $count -ge $idealCount ]; then
           ./logParser -cpp -log ${log} -out $combinedFile
           mv ${log}  ${log}_
        fi
    fi
  done
done
done
done
done
