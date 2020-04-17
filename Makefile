#GLDA
# GPU architecture specification
GPU_ARCH_FLAG   = arch=compute_70,code=sm_70

# CUDA compiler configuration
NVCC_HOME       = /usr/local/cuda
NVCC            = nvcc
CUDA_INC        = -I$(NVCC_HOME)/include
CUDA_LIB        = -L$(NVCC_HOME)/lib64 -lcuda -lcudart
CUDA_FLAGS      = -O3 -m64 -gencode $(GPU_ARCH_FLAG)
GLDAFLAGS	= -O3 -std=c++11
GLDAOBJS=	glda_dataset.o glda_utils.o glda_model.o glda_CUDASampling.o glda_sample_kernel.o

#PLDA
PLDAFLAGS	= -O3 -Wall -Wno-sign-compare
PLDAOBJS=	plda_accumulative_model.o plda_cmd_flags.o plda_common.o plda_document.o plda_model.o plda_sampler.o


#WLDA
WLDAFLAGS = -fopenmp -march=native -DNDEBUG -g -rdynamic -lnuma $(LIBS_DIR)/WarpLDA/release/gflags/libgflags_nothreads.a
WLDA_COMMON= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/AdjList.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/Bigraph.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/clock.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/NumaArray.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/Vocab.cpp.o
WLDA_WARP= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/lda.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/warp.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/warplda.cpp.o
WLDA_FORMAT= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/format.dir/format.cpp.o
WLDA_INC=  -I$(LIBS_DIR)/WarpLDA/release/gflags/include/ -I$(LIBS_DIR)/WarpLDA/./src#/gflags/gflags.h WarpLDA/release/gflags/include/gflags/gflags_declare.h WarpLDA/release/gflags/include/gflags/gflags_gflags.h


#BLDA
BLDA_OBJS= $(LIBS_DIR)/BleiLDA/lda-alpha.o $(LIBS_DIR)/BleiLDA/lda-data.o $(LIBS_DIR)/BleiLDA/lda-inference.o $(LIBS_DIR)/BleiLDA/utils.o $(LIBS_DIR)/BleiLDA/lda-estimate.o $(LIBS_DIR)/BleiLDA/lda-model.o


# C++ compiler configuration
CXX				= g++
CXXFLAGS	= -O3 -Wall -std=c++11 $(WLDAFLAGS)




# Project configuration
INCLUDE		= $(CUDA_INC)
LIB			= $(CUDA_LIB)
SRC_DIR= src
LIBS_DIR= $(SRC_DIR)/LDA_Libraries

OBJS=		main.o commons.o logger.o config.o document.o cluster.o myutils.o scanner.o dataProvenance.o geneticAlgorithm.o resultStatistics.o populationConfig.o parallelizables.o TopicModelling.o preProcessing.o wordFilter.o strtokenizer.o timer.o



MAIN=           provenance

all: $(OBJS) $(GLDAOBJS) $(PLDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(GLDAOBJS) $(PLDAOBJS) $(BLDA_OBJS) $(WLDA_COMMON) $(WLDA_WARP) $(WLDA_FORMAT) $(WLDA_INC) ${LIB} ${CXXFLAGS}

no_cuda: $(OBJS) $(PLDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(PLDAOBJS) $(BLDA_OBJS) $(WLDA_INC) ${CXXFLAGS}

main.o: $(SRC_DIR)/main.cpp
	$(CXX) -c -o main.o $(SRC_DIR)/main.cpp $(WLDA_INC) $(CXXFLAGS)

config.o: $(SRC_DIR)/config.cpp
	$(CXX) -c -o config.o $(SRC_DIR)/config.cpp $(CXXFLAGS)

commons.o: $(SRC_DIR)/commons.cpp
	$(CXX) -c -o commons.o $(SRC_DIR)/commons.cpp $(CXXFLAGS)

timer.o: $(SRC_DIR)/timer.cpp
		$(CXX) -c -o timer.o $(SRC_DIR)/timer.cpp $(CXXFLAGS)

logger.o: $(SRC_DIR)/logger.cpp
	$(CXX) -c -o logger.o $(SRC_DIR)/logger.cpp $(CXXFLAGS)

document.o: $(SRC_DIR)/document.cpp
	$(CXX) -c -o document.o $(SRC_DIR)/document.cpp $(CXXFLAGS)

cluster.o: $(SRC_DIR)/cluster.cpp
	$(CXX) -c -o cluster.o $(SRC_DIR)/cluster.cpp $(CXXFLAGS)

myutils.o: $(SRC_DIR)/utils.cpp
	$(CXX) -c -o myutils.o $(SRC_DIR)/utils.cpp $(CXXFLAGS)

scanner.o: $(SRC_DIR)/scanner.cpp
	$(CXX) -c -o scanner.o $(SRC_DIR)/scanner.cpp $(CXXFLAGS)

dataProvenance.o: $(SRC_DIR)/dataProvenance.cpp
	$(CXX) -c -o dataProvenance.o $(SRC_DIR)/dataProvenance.cpp $(WLDA_INC) $(CXXFLAGS)

geneticAlgorithm.o: $(SRC_DIR)/geneticAlgorithm.cpp
	$(CXX) -c -o geneticAlgorithm.o $(SRC_DIR)/geneticAlgorithm.cpp $(WLDA_INC) $(CXXFLAGS)

resultStatistics.o: $(SRC_DIR)/resultStatistics.cpp
	$(CXX) -c -o resultStatistics.o $(SRC_DIR)/resultStatistics.cpp $(CXXFLAGS)

populationConfig.o: $(SRC_DIR)/populationConfig.cpp
	$(CXX) -c -o populationConfig.o $(SRC_DIR)/populationConfig.cpp $(CXXFLAGS)

parallelizables.o: $(SRC_DIR)/parallelizables.cpp
	$(CXX) -c -o parallelizables.o $(SRC_DIR)/parallelizables.cpp $(CXXFLAGS)

TopicModelling.o: $(SRC_DIR)/TopicModelling.cpp
ifdef NO_CUDA
	$(CXX) -c -o TopicModelling.o $(SRC_DIR)/TopicModelling.cpp $(CXXFLAGS) $(WLDA_INC)
else
	$(CXX) -c -o TopicModelling.o $(SRC_DIR)/TopicModelling.cpp -D USECUDA $(CXXFLAGS) $(WLDA_INC) $(INCLUDE)
endif


preProcessing.o: $(SRC_DIR)/preProcessing.cpp
	$(CXX) -c -o preProcessing.o $(SRC_DIR)/preProcessing.cpp $(CXXFLAGS) $(WLDA_INC)

wordFilter.o: $(SRC_DIR)/wordFilter.cpp
	$(CXX) -c -o wordFilter.o $(SRC_DIR)/wordFilter.cpp $(CXXFLAGS)

strtokenizer.o: $(LIBS_DIR)/gldaCuda/src/strtokenizer.h $(LIBS_DIR)/gldaCuda/src/strtokenizer.cpp
	$(CXX) -c -o strtokenizer.o $(LIBS_DIR)/gldaCuda/src/strtokenizer.cpp $(CXXFLAGS) $(INCLUDE)

glda_dataset.o: $(LIBS_DIR)/gldaCuda/src/dataset.h $(LIBS_DIR)/gldaCuda/src/dataset.cpp
	$(CXX) -c -o glda_dataset.o $(LIBS_DIR)/gldaCuda/src/dataset.cpp $(GLDAFLAGS) $(INCLUDE)

glda_utils.o: $(LIBS_DIR)/gldaCuda/src/utils.h $(LIBS_DIR)/gldaCuda/src/utils.cpp
	$(CXX) -c -o glda_utils.o $(LIBS_DIR)/gldaCuda/src/utils.cpp $(GLDAFLAGS) $(INCLUDE)

glda_model.o: $(LIBS_DIR)/gldaCuda/src/model.h $(LIBS_DIR)/gldaCuda/src/model.cpp
	$(CXX) -c -o glda_model.o $(LIBS_DIR)/gldaCuda/src/model.cpp $(GLDAFLAGS) $(INCLUDE)

glda_CUDASampling.o: $(LIBS_DIR)/gldaCuda/src/CUDASampling.h $(LIBS_DIR)/gldaCuda/src/CUDASampling.cpp
	$(CXX) -c -o glda_CUDASampling.o $(LIBS_DIR)/gldaCuda/src/CUDASampling.cpp $(GLDAFLAGS) $(INCLUDE)

glda_sample_kernel.o: $(LIBS_DIR)/gldaCuda/src/sample_kernel.h $(LIBS_DIR)/gldaCuda/src/sample_kernel.cu
	$(NVCC) -c -o glda_sample_kernel.o $(LIBS_DIR)/gldaCuda/src/sample_kernel.cu $(INCLUDE) $(CUDA_FLAGS)

plda_accumulative_model.o: $(LIBS_DIR)/plda/accumulative_model.h $(LIBS_DIR)/plda/accumulative_model.cc
	$(CXX) -c -o plda_accumulative_model.o $(LIBS_DIR)/plda/accumulative_model.cc $(PLDAFLAGS)

plda_cmd_flags.o: $(LIBS_DIR)/plda/cmd_flags.h $(LIBS_DIR)/plda/cmd_flags.cc
	$(CXX) -c -o plda_cmd_flags.o $(LIBS_DIR)/plda/cmd_flags.cc $(PLDAFLAGS)

plda_common.o: $(LIBS_DIR)/plda/common.h $(LIBS_DIR)/plda/common.cc
	$(CXX) -c -o plda_common.o $(LIBS_DIR)/plda/common.cc $(PLDAFLAGS)

plda_document.o: $(LIBS_DIR)/plda/document.h $(LIBS_DIR)/plda/document.cc
	$(CXX) -c -o plda_document.o $(LIBS_DIR)/plda/document.cc $(PLDAFLAGS)

plda_model.o: $(LIBS_DIR)/plda/model.h $(LIBS_DIR)/plda/model.cc
	$(CXX) -c -o plda_model.o $(LIBS_DIR)/plda/model.cc $(PLDAFLAGS)

plda_sampler.o: $(LIBS_DIR)/plda/sampler.h $(LIBS_DIR)/plda/sampler.cc
	$(CXX) -c -o plda_sampler.o $(LIBS_DIR)/plda/sampler.cc $(PLDAFLAGS)

clean:
	rm $(OBJS)
	rm $(GLDAOBJS)
	rm $(PLDAOBJS)
	rm $(MAIN)
