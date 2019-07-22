# DataProvenance.LDA-GA
Version of the LDA-GA algorithm for data provenance in C++

## Building
To build the project use the makefile by incoking the command 'make' on the root directory
After building the project a execution file named 'ldaga' should be created. 
To clean the last build invoke make clean

## Running
To run the project for a specific population size and fitness threshold, use the following command:
```
./ldaga -p [population_size] -f [fitness_threshold]
```

To test the performance of the LDA algorithm testing different number of itterations and topics, use the following commang
```
./ldaga -metrics
```
