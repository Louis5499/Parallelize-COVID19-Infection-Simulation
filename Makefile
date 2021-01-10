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

omp-ver-block-time:
		$(CXX) $(CFLAGS) omp-ver-block-time.cc -o omp-ver-block-time

omp-ver-region-time:
		$(CXX) $(CFLAGS) omp-ver-region-time.cc -o omp-ver-region-time

run-seq:
	./sequential 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-block-time:
	srun -n1 -c12 ./omp-ver-block-time 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-region-time:
	srun -n1 -c12 ./omp-ver-region-time 100 50000 5000 720 720 3 3 0.99 2.5

run-omp-ver-block-time-exp:
	for number in 1 2 4 8 12 ; do \
			srun -n1 -c$$number ./omp-ver-block-time 100 50000 5000 720 720 3 3 0.99 2.5 ; \
	done

run-omp-ver-region-time-exp:
	for number in 1 2 4 8 12 ; do \
			srun -n1 -c$$number ./omp-ver-region-time 100 50000 5000 720 720 3 3 0.99 2.5 ; \
	done

clean:
	rm -rf Iteration-* sequential
