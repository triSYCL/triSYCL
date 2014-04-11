TARGETS = queue single_task
# The implementation use C++14
CXXFLAGS = -std=c++1y -I../include
# Assume Clang 3.5 at least. This may work with GCC 4.9 but not tested
CXX = clang++

all: $(TARGETS)

run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), ./$(command); )

clean:
	$(RM) $(TARGETS)
