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