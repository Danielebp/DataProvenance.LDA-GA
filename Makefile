# GPU architecture specification
GPU_ARCH_FLAG   = arch=compute_70,code=sm_70


# C++ compiler configuration
CXX             = g++
CXXFLAGS        = -O3 -std=c++11


# CUDA compiler configuration
NVCC_HOME       = /usr/local/cuda
NVCC            = nvcc
CUDA_INC        = -I$(NVCC_HOME)/include
CUDA_LIB        = -L$(NVCC_HOME)/lib64 -lcuda -lcudart
CUDA_FLAGS      = -O3 -m64 -gencode $(GPU_ARCH_FLAG)



# Project configuration
INCLUDE		= $(CUDA_INC)
LIB			= $(CUDA_LIB)


CUDAOBJS=	dataset.o utils.o model.o CUDASampling.o sample_kernel.o
OBJS=		main.o commons.o logger.o config.o document.o cluster.o myutils.o scanner.o dataProvenance.o geneticAlgorithm.o resultStatistics.o populationConfig.o parallelizables.o TopicModelling.o preProcessing.o wordFilter.o strtokenizer.o

MAIN=           main

all: $(OBJS) $(CUDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(CUDAOBJS) ${LIB} ${CXXFLAGS}

no_cuda: $(OBJS)
	$(CXX) -o $(MAIN) $(OBJS) ${CXXFLAGS}

main.o: ./main.cpp
	$(CXX) -c -o main.o ./main.cpp $(CXXFLAGS) $(INCLUDE)

config.o: ./config.cpp
	$(CXX) -c -o config.o ./config.cpp $(CXXFLAGS) $(INCLUDE)

commons.o: ./commons.cpp
	$(CXX) -c -o commons.o ./commons.cpp $(CXXFLAGS) $(INCLUDE)

logger.o: ./logger.cpp
	$(CXX) -c -o logger.o ./logger.cpp $(CXXFLAGS) $(INCLUDE)

document.o: ./document.cpp
	$(CXX) -c -o document.o ./document.cpp $(CXXFLAGS) $(INCLUDE)

cluster.o: ./cluster.cpp
	$(CXX) -c -o cluster.o ./cluster.cpp $(CXXFLAGS) $(INCLUDE)

myutils.o: ./utils.cpp
	$(CXX) -c -o myutils.o ./utils.cpp $(CXXFLAGS) $(INCLUDE)

scanner.o: ./scanner.cpp
	$(CXX) -c -o scanner.o ./scanner.cpp $(CXXFLAGS) $(INCLUDE)

dataProvenance.o: ./dataProvenance.cpp
	$(CXX) -c -o dataProvenance.o ./dataProvenance.cpp $(CXXFLAGS) $(INCLUDE)

geneticAlgorithm.o: ./geneticAlgorithm.cpp
	$(CXX) -c -o geneticAlgorithm.o ./geneticAlgorithm.cpp $(CXXFLAGS) $(INCLUDE)

resultStatistics.o: ./resultStatistics.cpp
	$(CXX) -c -o resultStatistics.o ./resultStatistics.cpp $(CXXFLAGS) $(INCLUDE)

populationConfig.o: ./populationConfig.cpp
	$(CXX) -c -o populationConfig.o ./populationConfig.cpp $(CXXFLAGS) $(INCLUDE)

parallelizables.o: ./parallelizables.cpp
	$(CXX) -c -o parallelizables.o ./parallelizables.cpp $(CXXFLAGS) $(INCLUDE)

TopicModelling.o: ./TopicModelling.cpp
ifdef NO_CUDA
	$(CXX) -c -o TopicModelling.o ./TopicModelling.cpp $(CXXFLAGS)
else
	$(CXX) -c -o TopicModelling.o ./TopicModelling.cpp -D USECUDA $(CXXFLAGS) $(INCLUDE)
endif


preProcessing.o: ./preProcessing.cpp
	$(CXX) -c -o preProcessing.o ./preProcessing.cpp $(CXXFLAGS) $(INCLUDE)

wordFilter.0: ./wordFilter.cpp
	$(CXX) -c -o wordFilter.o ./wordFilter.cpp $(CXXFLAGS) $(INCLUDE)

strtokenizer.o: ./gldaCuda/src/strtokenizer.h ./gldaCuda/src/strtokenizer.cpp
	$(CXX) -c -o strtokenizer.o ./gldaCuda/src/strtokenizer.cpp $(CXXFLAGS) $(INCLUDE)

dataset.o: ./gldaCuda/src/dataset.h ./gldaCuda/src/dataset.cpp
	$(CXX) -c -o dataset.o ./gldaCuda/src/dataset.cpp $(CXXFLAGS) $(INCLUDE)

utils.o: ./gldaCuda/src/utils.h ./gldaCuda/src/utils.cpp
	$(CXX) -c -o utils.o ./gldaCuda/src/utils.cpp $(CXXFLAGS) $(INCLUDE)

model.o: ./gldaCuda/src/model.h ./gldaCuda/src/model.cpp
	$(CXX) -c -o model.o ./gldaCuda/src/model.cpp $(CXXFLAGS) $(INCLUDE)

CUDASampling.o: ./gldaCuda/src/CUDASampling.h ./gldaCuda/src/CUDASampling.cpp
	$(CXX) -c -o CUDASampling.o ./gldaCuda/src/CUDASampling.cpp $(CXXFLAGS) $(INCLUDE)

sample_kernel.o: ./gldaCuda/src/sample_kernel.h ./gldaCuda/src/sample_kernel.cu
	$(NVCC) -c ./gldaCuda/src/sample_kernel.cu $(INCLUDE) $(CUDA_FLAGS)


clean:
	rm $(OBJS)
	rm $(CUDAOBJS)
	rm $(MAIN)
