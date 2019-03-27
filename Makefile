CC=g++

CFLAGS=-std=c++11
OBJ_PATH = ./obj

all: ldaga

clean:
	rm -rf $(OBJ_PATH)
	rm -f ldaga

OBJ_SRCS := document.cpp utils.cpp scanner.cpp geneticAlgorithm.cpp resultStatistics.cpp populationConfig.cpp parallelizables.cpp TopicModelling.cpp wordFilter.cpp
ALL_OBJ = $(patsubst %.cpp, %.o, $(OBJ_SRCS))
PLDA_SRCS := cmd_flags.o common.o document.o model.o accumulative_model.o sampler.o
PLDA_OBJ = $(patsubst %.o, plda/obj/%.o, $(PLDA_SRCS))
OBJ = $(addprefix $(OBJ_PATH)/, $(ALL_OBJ)) $(PLDA_OBJ)

$(OBJ_PATH)/%.o: %.cpp
	@ mkdir -p $(OBJ_PATH)
	$(CC) -c $(CFLAGS) $< -o $@

ldaga: main.cpp $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $< -o $@
