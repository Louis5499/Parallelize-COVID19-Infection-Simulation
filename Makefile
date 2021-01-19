# Flag Definition
CXX = g++
CFLAGS = -lm -O3 -march=native -std=c++11 -fopenmp
CXXFLAGS = $(CFLAGS)

# Variable Definition
TARGETS = main

# Command
.PHONY: sequential		
sequential:
	$(CXX) $(CFLAGS) ./sequential-code/sequential.cc -o ./sequential-code/sequential

omp-ver-block:
		$(CXX) $(CFLAGS) ./openmp-block/omp-ver-block.cc -o ./openmp-block/omp-ver-block

omp-ver-region:
		$(CXX) $(CFLAGS) ./openmp-row-region/omp-ver-region.cc -o ./openmp-row-region/omp-ver-region

.PHONY: cuda-v2-debug
debug:
	nvcc -Xcompiler -fopenmp -g -G ./cuda/cuda-v2.cu -o ./cuda/cuda-v2-debug

.PHONY: cuda-v2
cuda-v2:
	nvcc -Xcompiler -fopenmp ./cuda/cuda-v2.cu -o ./cuda/cuda-v2

run-seq:
	./sequential-code/sequential 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-block:
	srun -n1 -c12 ./openmp-block/omp-ver-block 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-region:
	srun -n1 -c12 ./openmp-row-region/omp-ver-region 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-block-exp:
	for number in 1 2 4 8 12 ; do \
			srun -n1 -c$$number ./openmp-block/omp-ver-block 100 50000 5000 720 720 3 3 0.99 2.5 ; \
	done

run-omp-ver-region-exp:
	for number in 1 2 4 8 12 ; do \
			srun -n1 -c$$number ./openmp-row-region/omp-ver-region 100 50000 5000 720 720 3 3 0.99 2.5 ; \
	done

run-cuda-v2:
	./cuda/cuda-v2 100 50000 5000 720 720 3 3 0.99 2.5

clean:
	rm -rf Iteration-* sequential
