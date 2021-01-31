HOME=/home/yutongq
GUROBI = /opt/local/stow/gurobi901/linux64
SDSL=$(HOME)/lib/libsdsl.a
SORTLIB=$(HOME)/lib/libdivsufsort.a
SORTLIB_64=$(HOME)/lib/libdivsufsort64.a


CC = gcc
CXX = g++

INCLUDES = -g -O0 -I $(HOME)/include -I $(GUROBI)/include
CXXFLAGS = -std=c++11 $(INCLUDES)

LDADD = $(SDSL) $(SORTLIB) $(SORTLIB_64)
GUROBI_LIB = $(shell ls $(GUROBI)/lib/libgurobi[0-9][0-9].so | tail -n 1 | xargs basename -s .so | cut -b 4-)
LDLIBS = $(LDADD)
LDLIBS += -L $(GUROBI)/lib -lgurobi_g++5.2 -l$(GUROBI_LIB)
LDFLAGS += -Wl,-rpath,$(GUROBI)/lib

SRCS=src/RLZ.hpp src/RLZ.cpp src/RLZGraph.hpp src/RLZGraph.cpp src/rlzmain.cpp

all: bin/rlzgraph 

bin/rlzgraph: $(subst .cpp,.o,$(SRCS))
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f bin/rlzgraph src/*.o
