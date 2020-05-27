# DataProvenance.LDA-GA
Version of the LDA-GA algorithm for data provenance in C++

## Requirements
- NVIDA GPU
- CUDA installation & nvcc
- lnuma library
- gcc

## Building
1. Edit the Makefile to reflect the CUDA compute capability of your machine

2. Run the `build.sh` script to build the solution and its dependencies. If the build succeeds the `provenance` executable will be created

3. Use the `make` command to recompile files after any change


## Running
To run the project for a specific population size and fitness threshold, use the following command:
```
./provenance config.json 
```

To test the performance of the LDA algorithm testing different number of itterations and topics, use the following commang
```
./provenance -metrics
```
