# Flag Definition
CXX = g++
CFLAGS = -lm -O3 -march=native -std=c++11 -fopenmp
CXXFLAGS = $(CFLAGS)

# Variable Definition
TARGETS = main

# Command
.PHONY: sequential		
sequential:
	$(CXX) $(CFLAGS) sequential.cc -o sequential

omp-ver-block:
		$(CXX) $(CFLAGS) omp-ver-block.cc -o omp-ver-block

omp-ver-region:
		$(CXX) $(CFLAGS) omp-ver-region.cc -o omp-ver-region

.PHONY: cuda-v2-debug
debug:
	nvcc -Xcompiler -fopenmp -g -G cuda-v2.cu -o cuda-v2-debug

.PHONY: cuda-v2
cuda-v2:
	nvcc -Xcompiler -fopenmp cuda-v2.cu -o cuda-v2

run-seq:
	./sequential 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-block:
	srun -n1 -c12 ./omp-ver-block 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-region:
	srun -n1 -c12 ./omp-ver-region 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-block-exp:
	for number in 1 2 4 8 12 ; do \
			srun -n1 -c$$number ./omp-ver-block 100 50000 5000 720 720 3 3 0.99 2.5 ; \
	done

run-omp-ver-region-exp:
	for number in 1 2 4 8 12 ; do \
			srun -n1 -c$$number ./omp-ver-region 100 50000 5000 720 720 3 3 0.99 2.5 ; \
	done

run-cuda-v2:
	./cuda-v2 100 50000 5000 720 720 3 3 0.99 2.5

clean:
	rm -rf Iteration-* sequential
