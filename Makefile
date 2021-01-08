# Flag Definition
CXX = g++
CFLAGS = -lm -O3 -march=native -std=c++11
CXXFLAGS = $(CFLAGS)

# Variable Definition
TARGETS = main

# Command
.PHONY: sequential		
sequential:
	$(CXX) $(CFLAGS) sequential.cc -o sequential

omp-ver:
		$(CXX) $(CFLAGS) omp-ver.cc -o omp-ver

run-seq:
	./sequential 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver:
	./omp-ver 100 50000 5000 720 720 3 3 0.99 2.5

clean:
	rm -rf Iteration-* sequential
