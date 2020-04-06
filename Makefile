# LightLDA

PROJECT := $(shell readlink $(dir $(lastword $(MAKEFILE_LIST))) -f)

LLDAFLAGS = -Wno-sign-compare \
           -fno-omit-frame-pointer

MULTIVERSO_DIR = $(PROJECT)/LightLDA/multiverso
MULTIVERSO_INC = $(MULTIVERSO_DIR)/include
MULTIVERSO_LIB = $(MULTIVERSO_DIR)/lib
THIRD_PARTY_LIB = $(MULTIVERSO_DIR)/third_party/lib
THIRD_PARTY_INC = $(MULTIVERSO_DIR)/third_party/include

INC_FLAGS = -I$(MULTIVERSO_INC) -I$(THIRD_PARTY_INC) -I$(PROJECT)/LightLDA/src
LD_FLAGS  = -L$(MULTIVERSO_LIB) -lmultiverso
LD_FLAGS += -L$(THIRD_PARTY_LIB) -lzmq -lmpich -lmpl -lpthread

BASE_SRC = $(shell find $(PROJECT)/LightLDA/src -type f -name "*.cpp" -type f ! -name "lightldamain.cpp")
BASE_OBJ = $(BASE_SRC:.cpp=.o)

LIGHTLDA_HEADERS = $(shell find $(PROJECT)/LightLDA/src -type f -name "*.h")
LIGHTLDA_SRC     = $(shell find $(PROJECT)/LightLDA/src -type f -name "*.cpp")
LIGHTLDA_OBJ = $(LIGHTLDA_SRC:.cpp=.o)

DUMP_BINARY_SRC = $(shell find $(PROJECT)/LightLDA/preprocess -type f -name "*.cpp")

BIN_DIR = $(PROJECT)/LightLDA/bin
DUMP_BINARY = $(BIN_DIR)/dump_binary


#$(LIGHTLDA): $(LIGHTLDA_OBJ)
#        $(CXX) $(LIGHTLDA_OBJ) $(LLDAFLAGS) $(INC_FLAGS) $(LD_FLAGS) -o $@

#$(LIGHTLDA_OBJ): %.o: %.cpp $(LIGHTLDA_HEADERS) $(MULTIVERSO_INC)
#        $(CXX) $(LLDAFLAGS) $(INC_FLAGS) -c $< -o $@



# GPU architecture specification
GPU_ARCH_FLAG   = arch=compute_70,code=sm_70


# C++ compiler configuration
CXX             = g++
CXXFLAGS        = -O0 -g -Wall -std=c++11 $(LLDAFLAGS) $(INC_FLAGS) $(LD_FLAGS)
PLDAFLAGS	= -O0 -g -Wall -Wno-sign-compare
GLDAFLAGS        = -O0 -g -std=c++11

# CUDA compiler configuration
NVCC_HOME       = /usr/local/cuda
NVCC            = nvcc
CUDA_INC        = -I$(NVCC_HOME)/include
CUDA_LIB        = -L$(NVCC_HOME)/lib64 -lcuda -lcudart
CUDA_FLAGS      = -O3 -m64 -gencode $(GPU_ARCH_FLAG)


# Project configuration
INCLUDE		= $(CUDA_INC)
LIB			= $(CUDA_LIB)


GLDAOBJS=	glda_dataset.o glda_utils.o glda_model.o glda_CUDASampling.o glda_sample_kernel.o
PLDAOBJS=	plda_accumulative_model.o plda_cmd_flags.o plda_common.o plda_document.o plda_model.o plda_sampler.o
OBJS=		main.o commons.o logger.o config.o document.o cluster.o myutils.o scanner.o dataProvenance.o geneticAlgorithm.o resultStatistics.o populationConfig.o parallelizables.o TopicModelling.o preProcessing.o wordFilter.o strtokenizer.o
BLDA_OBJS= BleiLDA/lda-alpha.o BleiLDA/lda-data.o BleiLDA/lda-inference.o BleiLDA/utils.o BleiLDA/lda-estimate.o BleiLDA/lda-model.o

LIGHTLDAOBJS=  LightLDA/bin/dump_binary LightLDA/src/alias_table.o LightLDA/src/document.o LightLDA/src/meta.o LightLDA/src/common.o LightLDA/src/eval.o LightLDA/src/model.o LightLDA/src/data_block.o LightLDA/src/sampler.o LightLDA/src/data_stream.o LightLDA/src/lightlda.o LightLDA/src/trainer.o LightLDA/multiverso/src/multiverso/aggregator.o LightLDA/multiverso/src/multiverso/barrier.o LightLDA/multiverso/src/multiverso/communicator.o LightLDA/multiverso/src/multiverso/data_block.o LightLDA/multiverso/src/multiverso/delta_pool.o LightLDA/multiverso/src/multiverso/endpoint_list.o LightLDA/multiverso/src/multiverso/lock.o LightLDA/multiverso/src/multiverso/log.o LightLDA/multiverso/src/multiverso/mpi_util.o LightLDA/multiverso/src/multiverso/msg_pack.o LightLDA/multiverso/src/multiverso/multiverso.o LightLDA/multiverso/src/multiverso/parameter_loader.o LightLDA/multiverso/src/multiverso/row_iter.o LightLDA/multiverso/src/multiverso/row.o LightLDA/multiverso/src/multiverso/server.o LightLDA/multiverso/src/multiverso/stop_watch.o LightLDA/multiverso/src/multiverso/table_iter.o LightLDA/multiverso/src/multiverso/table.o LightLDA/multiverso/src/multiverso/trainer.o LightLDA/multiverso/src/multiverso/vector_clock.o LightLDA/multiverso/src/multiverso/zmq_util.o

MAIN=           main

all: $(OBJS) $(GLDAOBJS) $(PLDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(GLDAOBJS) $(PLDAOBJS) $(BLDA_OBJS) $(LIGHTLDAOBJS) ${LIB} ${CXXFLAGS}

no_cuda: $(OBJS) $(PLDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(PLDAOBJS) $(BLDA_OBJS) ${CXXFLAGS}

main.o: ./main.cpp
	$(CXX) -c -o main.o ./main.cpp $(CXXFLAGS)

config.o: ./config.cpp
	$(CXX) -c -o config.o ./config.cpp $(CXXFLAGS)

commons.o: ./commons.cpp
	$(CXX) -c -o commons.o ./commons.cpp $(CXXFLAGS)

logger.o: ./logger.cpp
	$(CXX) -c -o logger.o ./logger.cpp $(CXXFLAGS)

document.o: ./document.cpp
	$(CXX) -c -o document.o ./document.cpp $(CXXFLAGS)

cluster.o: ./cluster.cpp
	$(CXX) -c -o cluster.o ./cluster.cpp $(CXXFLAGS)

myutils.o: ./utils.cpp
	$(CXX) -c -o myutils.o ./utils.cpp $(CXXFLAGS)

scanner.o: ./scanner.cpp
	$(CXX) -c -o scanner.o ./scanner.cpp $(CXXFLAGS)

dataProvenance.o: ./dataProvenance.cpp
	$(CXX) -c -o dataProvenance.o ./dataProvenance.cpp $(CXXFLAGS)

geneticAlgorithm.o: ./geneticAlgorithm.cpp
	$(CXX) -c -o geneticAlgorithm.o ./geneticAlgorithm.cpp $(CXXFLAGS)

resultStatistics.o: ./resultStatistics.cpp
	$(CXX) -c -o resultStatistics.o ./resultStatistics.cpp $(CXXFLAGS)

populationConfig.o: ./populationConfig.cpp
	$(CXX) -c -o populationConfig.o ./populationConfig.cpp $(CXXFLAGS)

parallelizables.o: ./parallelizables.cpp
	$(CXX) -c -o parallelizables.o ./parallelizables.cpp $(CXXFLAGS)

TopicModelling.o: ./TopicModelling.cpp
ifdef NO_CUDA
	$(CXX) -c -o TopicModelling.o ./TopicModelling.cpp $(CXXFLAGS)
else
	$(CXX) -c -o TopicModelling.o ./TopicModelling.cpp -D USECUDA $(CXXFLAGS) $(INCLUDE)
endif


preProcessing.o: ./preProcessing.cpp
	$(CXX) -c -o preProcessing.o ./preProcessing.cpp $(CXXFLAGS)

wordFilter.0: ./wordFilter.cpp
	$(CXX) -c -o wordFilter.o ./wordFilter.cpp $(CXXFLAGS)

strtokenizer.o: ./gldaCuda/src/strtokenizer.h ./gldaCuda/src/strtokenizer.cpp
	$(CXX) -c -o strtokenizer.o ./gldaCuda/src/strtokenizer.cpp $(CXXFLAGS) $(INCLUDE)

glda_dataset.o: ./gldaCuda/src/dataset.h ./gldaCuda/src/dataset.cpp
	$(CXX) -c -o glda_dataset.o ./gldaCuda/src/dataset.cpp $(GLDAFLAGS) $(INCLUDE)

glda_utils.o: ./gldaCuda/src/utils.h ./gldaCuda/src/utils.cpp
	$(CXX) -c -o glda_utils.o ./gldaCuda/src/utils.cpp $(GLDAFLAGS) $(INCLUDE)

glda_model.o: ./gldaCuda/src/model.h ./gldaCuda/src/model.cpp
	$(CXX) -c -o glda_model.o ./gldaCuda/src/model.cpp $(GLDAFLAGS) $(INCLUDE)

glda_CUDASampling.o: ./gldaCuda/src/CUDASampling.h ./gldaCuda/src/CUDASampling.cpp
	$(CXX) -c -o glda_CUDASampling.o ./gldaCuda/src/CUDASampling.cpp $(GLDAFLAGS) $(INCLUDE)

glda_sample_kernel.o: ./gldaCuda/src/sample_kernel.h ./gldaCuda/src/sample_kernel.cu
	$(NVCC) -c -o glda_sample_kernel.o ./gldaCuda/src/sample_kernel.cu $(INCLUDE) $(CUDA_FLAGS)

plda_accumulative_model.o: ./plda/accumulative_model.h ./plda/accumulative_model.cc
	$(CXX) -c -o plda_accumulative_model.o ./plda/accumulative_model.cc $(PLDAFLAGS)

plda_cmd_flags.o: ./plda/cmd_flags.h ./plda/cmd_flags.cc
	$(CXX) -c -o plda_cmd_flags.o ./plda/cmd_flags.cc $(PLDAFLAGS)

plda_common.o: ./plda/common.h ./plda/common.cc
	$(CXX) -c -o plda_common.o ./plda/common.cc $(PLDAFLAGS)

plda_document.o: ./plda/document.h ./plda/document.cc
	$(CXX) -c -o plda_document.o ./plda/document.cc $(PLDAFLAGS)

plda_model.o: ./plda/model.h ./plda/model.cc
	$(CXX) -c -o plda_model.o ./plda/model.cc $(PLDAFLAGS)

plda_sampler.o: ./plda/sampler.h ./plda/sampler.cc
	$(CXX) -c -o plda_sampler.o ./plda/sampler.cc $(PLDAFLAGS)

clean:
	rm $(OBJS)
	rm $(GLDAOBJS)
	rm $(PLDAOBJS)
	rm $(MAIN)
