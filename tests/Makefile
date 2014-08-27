# Compute the absolute directory name from the location of this Makefile
# so that we can compile from anywhere even if we use make -f
# <this_makefile> ... as used by make check:
triSYCL_DIR=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Use all the .cpp files from the subdirectories
TARGETS = $(basename $(wildcard */*.cpp))

# The implementation uses C++14 and OpenMP
CXXFLAGS = -g -std=c++1y -I$(triSYCL_DIR)/include

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
	# The config file for triSYCL needs at least Python 3.3
	test "unset$$TRISYCL_LIT == unset" && echo 'Initialize TRISYCL_LIT variable to the path of "lit" command' ; echo 1
	python3 $$TRISYCL_LIT .

# A special target to be called as from the test as "make execute
# TARGET=%s" with the right Makefile. There is a short-cut in the lit.cfg
# to use "RUN: %{execute}%s | %{filecheck} %s" in test files instead.

# Add a dependency on the binary name, i.e. without the extension
execute: $(basename $(TARGET))
	# Execute the compiled binary
	$<
