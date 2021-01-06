# Flag Definition
CXX = g++
CFLAGS = -lm -O3 -march=native
CXXFLAGS = $(CFLAGS)

# Variable Definition
TARGETS = main

# Command
.PHONY: sequential
sequential:
		$(CXX) $(CFLAGS) sequential.cc -o sequential

run-seq:
	./sequential 100 50000 5000 720 720 3 3 0.99 2.5