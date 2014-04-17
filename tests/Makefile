TARGETS = queue single_task vector_add parallel_vector_add parallel_matrix_add

# The implementation uses C++14
CXXFLAGS = -g -std=c++1y -I../include

# Assume Clang 3.5 at least
# There are still some limitations with "auto" return type methods that
# does not work with "-g" option
CXX = clang++
# This work with GCC 4.9 too
CXX = g++-4.9

all: $(TARGETS)

run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), ./$(command); )

clean:
	$(RM) $(TARGETS)
