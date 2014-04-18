TARGETS = queue single_task vector_add parallel_vector_add \
	parallel_matrix_add \
	$(basename $(wildcard 2014-04-21-HPC-GPU_Meetup/*.cpp))

# The implementation uses C++14 and OpenMP
CXXFLAGS = -g -std=c++1y -I../include

# To use OpenMP to execute SYCL kernels:
CXXFLAGS += -fopenmp

# Assume Clang 3.5 at least
# There are still some limitations in Clang:
# - no OpenMP;
# - the "auto" return type methods that does not work with "-g" option
#CXX = clang++

# But everything works fine with GCC 4.9!
CXX = g++-4.9

all: $(TARGETS)

run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), echo; echo $(command):; ./$(command); )

clean:
	$(RM) $(TARGETS)
