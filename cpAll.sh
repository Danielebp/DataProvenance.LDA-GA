#!/bin/bash

dataSize=("10" "20" "50" "100" "150" "200" "250")

for size in ${dataSize[*]}; do
for fit in {10..95..10}; do
for it in {1..10};do
  cp -r ./data/data_${size}/new_cuda_${size}art_${fit}fit_${it}it/execution* $1/execution_cuda_${size}art_${fit}fit_${it}it.log
done
done
done


