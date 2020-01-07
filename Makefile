SRCS=src/main.cpp src/RLZgraph.cpp src/RLZfact.cpp src/bitvector.cpp
TESTSRCS=test/test.cpp src/RLZgraph.cpp src/RLZfact.cpp src/bitvector.cpp

CXX = g++ -std=c++11
LDLIBS = -lz -lm

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