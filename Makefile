HOME=/home/yutongq
GUROBI = /opt/local/stow/gurobi901/linux64

CC = gcc
CXX = g++

INCLUDES = -g -O0 -I $(HOME)/include -I $(GUROBI)/include
CXXFLAGS = -std=c++11 $(INCLUDES)

LDADD = $(HOME)/lib/libsdsl.a $(HOME)/lib/libdivsufsort.a $(HOME)/lib/libdivsufsort64.a
GUROBI_LIB = $(shell ls $(GUROBI)/lib/libgurobi[0-9][0-9].so | tail -n 1 | xargs basename -s .so | cut -b 4-)
LDLIBS = $(LDADD)
LDLIBS += -L $(GUROBI)/lib -lgurobi_g++5.2 -l$(GUROBI_LIB)
LDFLAGS += -Wl,-rpath,$(GUROBI)/lib

SRCS=src/RLZ.hpp src/RLZ.cpp src/RLZGraph.hpp src/RLZGraph.cpp src/main.cpp
TESTSRCS=src/RLZ.hpp src/RLZ.cpp test/test.cpp
printSASRC=helpers/printSA.cpp
all: bin/main 

test: bin/test

altmain:bin/altmain

printSA: bin/printSA

bin/main: $(subst .cpp,.o,$(SRCS))
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

	
bin/altmain: $(subst .cpp,.o,$(SRCS))
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# bin/test: $(subst .cpp,.o,$(TESTSRCS))
# 	mkdir -p bin
# 	$(CXX) -o $@ $^ $(LDLIBS)

bin/test: $(subst .cpp,.o,$(TESTSRCS))
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

bin/printSA: $(subst .cpp,.o,$(printSASRC))
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f bin/main src/*.o test/*.o
