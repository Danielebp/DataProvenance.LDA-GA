###################### GLDA ####################
# GPU architecture specification
GPU_ARCH_FLAG   = arch=compute_70,code=sm_70
GLDAFLAGS	= -O3 -std=c++11

# CUDA compiler configuration
NVCC_HOME       = /usr/local/cuda
NVCC            = nvcc
CUDA_INC        = -I$(NVCC_HOME)/include
CUDA_LIB        = -L$(NVCC_HOME)/lib64 -lcuda -lcudart
CUDA_FLAGS      = -O3 -m64 -gencode $(GPU_ARCH_FLAG)
GLDAOBJS=	$(OBJ_DIR)/glda_dataset.o $(OBJ_DIR)/glda_utils.o $(OBJ_DIR)/glda_model.o $(OBJ_DIR)/glda_CUDASampling.o $(OBJ_DIR)/glda_sample_kernel.o


###################### PLDA #####################
PLDAFLAGS	= -g -O0 -Wall -Wno-sign-compare -std=c++11
PLDAOBJS=	$(OBJ_DIR)/plda_accumulative_model.o $(OBJ_DIR)/plda_cmd_flags.o $(OBJ_DIR)/plda_common.o $(OBJ_DIR)/plda_document.o $(OBJ_DIR)/plda_model.o $(OBJ_DIR)/plda_sampler.o


###################### WLDA #####################
WLDAFLAGS = -fopenmp -march=native -DNDEBUG -g -rdynamic -lnuma $(LIBS_DIR)/WarpLDA/release/gflags/libgflags_nothreads.a
WLDA_COMMON= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/AdjList.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/Bigraph.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/clock.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/NumaArray.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/Vocab.cpp.o
WLDA_WARP= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/lda.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/warp.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/warplda.cpp.o
WLDA_FORMAT= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/format.dir/format.cpp.o
WLDA_OBJS= $(WLDA_COMMON) $(WLDA_WARP) $(WLDA_FORMAT)
WLDA_INC=  -I$(LIBS_DIR)/WarpLDA/release/gflags/include/ -I$(LIBS_DIR)/WarpLDA/./src#/gflags/gflags.h $(LIBS_DIR)/WarpLDA/release/gflags/include/gflags/gflags_declare.h $(LIBS_DIR)/WarpLDA/release/gflags/include/gflags/gflags_gflags.h


###################### BLDA ####################
BLDA_OBJS= $(LIBS_DIR)/BleiLDA/lda-alpha.o $(LIBS_DIR)/BleiLDA/lda-data.o $(LIBS_DIR)/BleiLDA/lda-inference.o $(LIBS_DIR)/BleiLDA/utils.o $(LIBS_DIR)/BleiLDA/lda-estimate.o $(LIBS_DIR)/BleiLDA/lda-model.o


#################### GENERAL ###################

# C++ compiler configuration
CXX				= g++
CXXFLAGS	= -O0 -g -Wall -std=c++11 $(WLDAFLAGS) 


# Project configuration
SRC_DIR= ./src
LIBS_DIR= ./src/LDA_Libraries
OBJ_DIR= ./obj
OBJS=		$(OBJ_DIR)/main.o $(OBJ_DIR)/commons.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/config.o $(OBJ_DIR)/document.o $(OBJ_DIR)/cluster.o $(OBJ_DIR)/myutils.o $(OBJ_DIR)/scanner.o $(OBJ_DIR)/dataProvenance.o $(OBJ_DIR)/geneticAlgorithm.o $(OBJ_DIR)/resultStatistics.o $(OBJ_DIR)/populationConfig.o $(OBJ_DIR)/parallelizables.o $(OBJ_DIR)/TopicModelling.o $(OBJ_DIR)/preProcessing.o $(OBJ_DIR)/wordFilter.o $(OBJ_DIR)/strtokenizer.o

MAIN=  provenance
LIBS_OBJS= $(GLDAOBJS) $(PLDAOBJS) $(BLDA_OBJS) $(WLDA_OBJS)


# Rules

all: directories $(OBJS) $(GLDAOBJS) $(PLDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(LIBS_OBJS) $(WLDA_INC) ${CUDA_LIB} ${CXXFLAGS} 

no_cuda: $(OBJS) $(PLDAOBJS)
	$(CXX) -o $(MAIN) $(OBJS) $(PLDAOBJS) $(BLDA_OBJS) $(WLDA_OBJS) $(WLDA_INC) ${CXXFLAGS}

.PHONY: directories
directories: $(OBJ_DIR)/

$(OBJ_DIR)/:
		mkdir -p $@

###################### Main Flow #####################
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/main.cpp $(WLDA_INC) $(CXXFLAGS)

$(OBJ_DIR)/dataProvenance.o: $(SRC_DIR)/dataProvenance.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/dataProvenance.cpp $(WLDA_INC) $(CXXFLAGS)

$(OBJ_DIR)/geneticAlgorithm.o: $(SRC_DIR)/geneticAlgorithm.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/geneticAlgorithm.cpp $(WLDA_INC) $(CXXFLAGS)

# ADAPTER
$(OBJ_DIR)/TopicModelling.o: $(SRC_DIR)/TopicModelling.cpp
ifdef NO_CUDA
	$(CXX) -c -o $@ $(SRC_DIR)/TopicModelling.cpp $(CXXFLAGS) $(WLDA_INC)
else
	$(CXX) -c -o $@ $(SRC_DIR)/TopicModelling.cpp -D USECUDA $(CXXFLAGS) $(WLDA_INC) $(CUDA_INC)
endif

$(OBJ_DIR)/preProcessing.o: $(SRC_DIR)/preProcessing.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/preProcessing.cpp $(CXXFLAGS) $(WLDA_INC)

$(OBJ_DIR)/cluster.o: $(SRC_DIR)/cluster.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/cluster.cpp $(CXXFLAGS)

$(OBJ_DIR)/resultStatistics.o: $(SRC_DIR)/resultStatistics.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/resultStatistics.cpp $(CXXFLAGS)

##################### AUXILIARS ##########################
$(OBJ_DIR)/config.o: $(SRC_DIR)/config.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/config.cpp $(CXXFLAGS)

$(OBJ_DIR)/commons.o: $(SRC_DIR)/commons.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/commons.cpp $(CXXFLAGS)

$(OBJ_DIR)/logger.o: $(SRC_DIR)/logger.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/logger.cpp $(CXXFLAGS)

$(OBJ_DIR)/document.o: $(SRC_DIR)/document.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/document.cpp $(CXXFLAGS)

$(OBJ_DIR)/myutils.o: $(SRC_DIR)/utils.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/utils.cpp $(CXXFLAGS)

$(OBJ_DIR)/scanner.o: $(SRC_DIR)/scanner.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/scanner.cpp $(CXXFLAGS)

$(OBJ_DIR)/populationConfig.o: $(SRC_DIR)/populationConfig.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/populationConfig.cpp $(CXXFLAGS)

$(OBJ_DIR)/wordFilter.o: $(SRC_DIR)/wordFilter.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/wordFilter.cpp $(CXXFLAGS)

$(OBJ_DIR)/strtokenizer.o: $(LIBS_DIR)/gldaCuda/src/strtokenizer.h $(LIBS_DIR)/gldaCuda/src/strtokenizer.cpp
	$(CXX) -c -o $@ $(LIBS_DIR)/gldaCuda/src/strtokenizer.cpp $(CXXFLAGS) $(CUDA_INC)

$(OBJ_DIR)/parallelizables.o: $(SRC_DIR)/parallelizables.cpp
	$(CXX) -c -o $@ $(SRC_DIR)/parallelizables.cpp $(CXXFLAGS)

######################### GLDA #########################

$(OBJ_DIR)/glda_dataset.o: $(LIBS_DIR)/gldaCuda/src/dataset.h $(LIBS_DIR)/gldaCuda/src/dataset.cpp
	$(CXX) -c -o $@ $(LIBS_DIR)/gldaCuda/src/dataset.cpp $(GLDAFLAGS) $(CUDA_INC)

$(OBJ_DIR)/glda_utils.o: $(LIBS_DIR)/gldaCuda/src/utils.h $(LIBS_DIR)/gldaCuda/src/utils.cpp
	$(CXX) -c -o $@ $(LIBS_DIR)/gldaCuda/src/utils.cpp $(GLDAFLAGS) $(CUDA_INC)

$(OBJ_DIR)/glda_model.o: $(LIBS_DIR)/gldaCuda/src/model.h $(LIBS_DIR)/gldaCuda/src/model.cpp
	$(CXX) -c -o $@ $(LIBS_DIR)/gldaCuda/src/model.cpp $(GLDAFLAGS) $(CUDA_INC)

$(OBJ_DIR)/glda_CUDASampling.o: $(LIBS_DIR)/gldaCuda/src/CUDASampling.h $(LIBS_DIR)/gldaCuda/src/CUDASampling.cpp
	$(CXX) -c -o $@ $(LIBS_DIR)/gldaCuda/src/CUDASampling.cpp $(GLDAFLAGS) $(CUDA_INC)

$(OBJ_DIR)/glda_sample_kernel.o: $(LIBS_DIR)/gldaCuda/src/sample_kernel.h $(LIBS_DIR)/gldaCuda/src/sample_kernel.cu
	$(NVCC) -c -o $@ $(LIBS_DIR)/gldaCuda/src/sample_kernel.cu $(CUDA_INC) $(CUDA_FLAGS)


######################## PLDA #########################
$(OBJ_DIR)/plda_accumulative_model.o: $(LIBS_DIR)/plda/accumulative_model.h $(LIBS_DIR)/plda/accumulative_model.cc
	$(CXX) -c -o $@ $(LIBS_DIR)/plda/accumulative_model.cc $(PLDAFLAGS)

$(OBJ_DIR)/plda_cmd_flags.o: $(LIBS_DIR)/plda/cmd_flags.h $(LIBS_DIR)/plda/cmd_flags.cc
	$(CXX) -c -o $@ $(LIBS_DIR)/plda/cmd_flags.cc $(PLDAFLAGS)

$(OBJ_DIR)/plda_common.o: $(LIBS_DIR)/plda/common.h $(LIBS_DIR)/plda/common.cc
	$(CXX) -c -o $@ $(LIBS_DIR)/plda/common.cc $(PLDAFLAGS)

$(OBJ_DIR)/plda_document.o: $(LIBS_DIR)/plda/document.h $(LIBS_DIR)/plda/document.cc
	$(CXX) -c -o $@ $(LIBS_DIR)/plda/document.cc $(PLDAFLAGS)

$(OBJ_DIR)/plda_model.o: $(LIBS_DIR)/plda/model.h $(LIBS_DIR)/plda/model.cc
	$(CXX) -c -o $@ $(LIBS_DIR)/plda/model.cc $(PLDAFLAGS)

$(OBJ_DIR)/plda_sampler.o: $(LIBS_DIR)/plda/sampler.h $(LIBS_DIR)/plda/sampler.cc
	$(CXX) -c -o $@ $(LIBS_DIR)/plda/sampler.cc $(PLDAFLAGS)

######################################################

clean:
	rm $(OBJS)
	rm $(GLDAOBJS)
	rm $(PLDAOBJS)
	rm $(MAIN)
