TARGETS = queue single_task vector_add

# The implementation use C++14
CXXFLAGS = -std=c++1y -I../include
# Assume Clang 3.5 at least
CXX = clang++
# This work with GCC 4.9 too
#CXX = g++-4.9

all: $(TARGETS)

run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), ./$(command); )

clean:
	$(RM) $(TARGETS)
