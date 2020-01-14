#BOOST=/home/yutongq/SQUID/Tools/boost_1_64_0/
BOOST=~/storage/Tools/boost_1_64_0/
# BAMTOOLS = /home/yutongq/SQUID/Tools/bamtools/usr/local/
# GLPK = /home/yutongq/SQUID/Tools/glpk-4.62/bin

CC = gcc
CXX = g++
# INCLUDES = -g -I $(BAMTOOLS)/include/bamtools -I $(GLPK)/include -I $(BOOST)
INCLUDES = -g -I $(BOOST)
CXXFLAGS = -std=c++11 $(INCLUDES)
# LDADD = $(BAMTOOLS)/lib/libbamtools.a $(GLPK)/lib/libglpk.a
LDLIBS = -lz -lm

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