#!/bin/bash

dataSize=("10" "20" "50" "100" "150" "200" "250")

for size in ${dataSize[*]}; do
for fit in {10..95..10}; do
for it in {1..10};do
  ./main config.json -f 0.${fit} -dir ./data/data_${size}/ -out new_cpp_${size}art_${fit}fit_${it}it
done
done
done


