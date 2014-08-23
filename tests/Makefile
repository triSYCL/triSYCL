TARGETS = queue single_task vector_add range id parallel_vector_add \
	parallel_matrix_add hierarchical \
	$(basename $(wildcard 2014-04-21-HPC-GPU_Meetup/*.cpp))

# The implementation uses C++14 and OpenMP
CXXFLAGS = -g -std=c++1y -I../include

# To use OpenMP to execute SYCL kernels:
CXXFLAGS += -fopenmp

# To enable debug and tracing:
#CXXFLAGS += -DTRISYCL_DEBUG

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

check:
	# Launch testing with lit tool from LLVM in current directory

	# lit can be found for example on Debian/Ubuntu in package
	# llvm-3.5-tools in /usr/lib/llvm-3.5/build/utils/lit/lit.py
	# so try before running the check:
	# export TRISYCL_LIT=/usr/lib/llvm-3.5/build/utils/lit/lit.py

	# Running the test requires also the FileCheck utility, which is also in
	# llvm-3.5-tools for example, but with a different name
	# /usr/bin/FileCheck-3.5
	# A quick-and-dirty hack is to do a
	# sudo ln -s FileCheck-3.5 /usr/bin/FileCheck
	test "unset$$TRISYCL_LIT == unset" && echo 'Initialize TRISYCL_LIT variable to the path of "lit" command' ; echo 1
	$$TRISYCL_LIT .

# A special target to be called as from the test as make execute TARGET=%s
# Add a dependency on the binary name, i.e. without the extension
execute: $(basename $(TARGET))
	# Execute the compiled binary
	$<
