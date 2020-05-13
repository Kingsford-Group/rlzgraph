#BOOST=/home/yutongq/SQUID/Tools/boost_1_64_0/
BOOST=~/storage/Tools/boost_1_64_0/
HOME=/home/yutongq
# BAMTOOLS = /home/yutongq/SQUID/Tools/bamtools/usr/local/
# GLPK = /home/yutongq/SQUID/Tools/glpk-4.62/bin
CC = gcc
CXX = g++
# INCLUDES = -g -I $(BAMTOOLS)/include/bamtools -I $(GLPK)/include -I $(BOOST)
INCLUDES = -g -I $(BOOST) -I $(HOME)/include/
CXXFLAGS = -std=c++11 $(INCLUDES)
LDADD = $(HOME)/lib/libsdsl.a $(HOME)/lib/libdivsufsort.a $(HOME)/lib/libdivsufsort64.a
# LDADD = $(BAMTOOLS)/lib/libbamtools.a $(GLPK)/lib/libglpk.a
LDLIBS = -lz -lm $(LDADD)

SRCS=src/main.cpp src/RLZgraph.cpp src/RLZfact.cpp src/SuffixTree.cpp
TESTSRCS=test/test.cpp src/RLZgraph.cpp src/RLZfact.cpp src/SuffixTree.cpp

all: bin/main 

test: bin/test

bin/main: $(subst .cpp,.o,$(SRCS))
	mkdir -p bin
	$(CXX) -o $@ $^ $(LDLIBS)

bin/test: $(subst .cpp,.o,$(TESTSRCS))
	mkdir -p bin
	$(CXX) -o $@ $^ $(LDLIBS)

clean:
	rm -f bin/main src/*.o test/*.o
