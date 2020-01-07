SRCS=src/main.cpp src/RLZgraph.cpp src/RLZfact.cpp

CXX = g++ -std=c++11
LDLIBS = -lz -lm

all: bin/main

bin/main: $(subst .cpp,.o,$(SRCS))
	mkdir -p bin
	$(CXX) -o $@ $^ $(LDLIBS)

clean:
	rm -f bin/main src/*.o