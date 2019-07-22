# DataProvenance.LDA-GA
Version of the LDA-GA algorithm for data provenance in C++

## Building
To build the project use the makefile by incoking the command 'make' on the root directory
To clean the last build invoke make clean

## Running
After building the project a execution file named 'ldaga' should be created. To run the project run that file using the following commands:
```
./ldaga -p [population_size] -f [fitness_threshold] [-metrics]
```
Where using the flag -metrics will output metrics along the execution.
