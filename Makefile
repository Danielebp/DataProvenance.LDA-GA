# *****************************************************
# Variables to control Makefile operation

CXX = g++
CXXFLAGS = -std=c++11

# ****************************************************
# Targets needed to bring the executable up to date

main: main.o document.o utils.o geneticAlgorithm.o resultStatistics.o populationConfig.o scanner.o parallelizables.o
	$(CXX) $(CXXFLAGS) -o ldaga main.o document.o utils.o geneticAlgorithm.o resultStatistics.o populationConfig.o scanner.o parallelizables.o

# ****************************************************
# Dependencies
main.o: main.cpp utils.h document.h geneticAlgorithm.h
	$(CXX) $(CXXFLAGS) -c main.cpp

document.o: document.h document.cpp
	$(CXX) $(CXXFLAGS) -c document.cpp

utils.o: utils.h utils.cpp
	$(CXX) $(CXXFLAGS) -c utils.cpp

scanner.o: scanner.h scanner.cpp
		$(CXX) $(CXXFLAGS) -c scanner.cpp

geneticAlgorithm.o: geneticAlgorithm.h populationConfig.h resultStatistics.h parallelizables.h
	$(CXX) $(CXXFLAGS) -c geneticAlgorithm.cpp

resultStatistics.o: resultStatistics.h populationConfig.h
	$(CXX) $(CXXFLAGS) -c resultStatistics.cpp

populationConfig.o: populationConfig.h utils.h
	$(CXX) $(CXXFLAGS) -c populationConfig.cpp

parallelizables.o: parallelizables.cpp parallelizables.h populationConfig.h scanner.h
	$(CXX) $(CXXFLAGS) -c parallelizables.cpp

clean:
	rm *.o ldaga
