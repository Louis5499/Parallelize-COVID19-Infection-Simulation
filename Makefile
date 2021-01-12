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

.PHONY: cuda-v2-debug
debug:
	nvcc -Xcompiler -fopenmp -g -G cuda-v2.cu -o cuda-v2-debug

.PHONY: cuda-v2
cuda-v2:
	nvcc -Xcompiler -fopenmp cuda-v2.cu -o cuda-v2

run-seq:
	./sequential 100 50000 5000 720 720 3 3 0.99 2.5

run-cuda-v2:
	./cuda-v2 100 50000 5000 720 720 3 3 0.99 2.5

clean:
	rm -rf Iteration-* sequential cuda-v2 cuda-v2-debug