CC=g++

CFLAGS=-std=c++11
OBJ_PATH = ./obj

all: ldaga

clean:
	rm -rf $(OBJ_PATH)
	rm -f ldaga

OBJ_SRCS := document.cpp cluster.cpp utils.cpp scanner.cpp dataProvenance.cpp geneticAlgorithm.cpp resultStatistics.cpp populationConfig.cpp parallelizables.cpp TopicModelling.cpp wordFilter.cpp
ALL_OBJ = $(patsubst %.cpp, %.o, $(OBJ_SRCS))
PLDA_SRCS := dataset.o model.o strtokenizer.o utils.o
PLDA_OBJ = $(patsubst %.o, GibbsLDA/src/%.o, $(PLDA_SRCS))
OBJ = $(addprefix $(OBJ_PATH)/, $(ALL_OBJ)) $(PLDA_OBJ)

$(OBJ_PATH)/%.o: %.cpp
	@ mkdir -p $(OBJ_PATH)
	$(CC) -c $(CFLAGS) $< -o $@

ldaga: main.cpp $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $< -o $@

TESTOBJ_PATH = ./test
TESTOBJ_SRCS := scanner.cpp cluster.cpp document.cpp utils.cpp
TESTALL_OBJ = $(patsubst %.cpp, %.o, $(TESTOBJ_SRCS))
TESTOBJ = $(addprefix $(TESTOBJ_PATH)/, $(TESTALL_OBJ))

$(TESTOBJ_PATH)/%.o: %.cpp
	@ mkdir -p $(TESTOBJ_PATH)
	$(CC) -c $(CFLAGS) $< -o $@

test: test.cpp $(TESTALL_OBJ)
	$(CC) $(CFLAGS) $(TESTALL_OBJ) $< -o $@
