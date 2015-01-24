# Compute the absolute directory name from the location of this Makefile
# so that we can compile from anywhere even if we use make -f
# <this_makefile> ... as used by make check:
triSYCL_DIR=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Use all the .cpp files from the subdirectories
TARGETS = $(basename $(wildcard */*.cpp))

# The implementation uses C++14 and OpenMP
CXXFLAGS = -Wall -std=c++1y -I$(triSYCL_DIR)/include \
	-I$(triSYCL_DIR)/tests/common

# To use OpenMP to execute SYCL kernels:
CXXFLAGS += -fopenmp

# To avoid asynchronous kernels
CXXFLAGS += -DTRISYCL_ASYNC=0

# To enable debug and tracing:
#CXXFLAGS += -g -DTRISYCL_DEBUG

# By default run lit checking in the current directory
CHECKING_DIR ?= .

# Set default compiler instead of default CXX=g++ value

# Assume Clang 3.6 at least
# There are still some limitations in Clang:
# - no OpenMP;
# - the "auto" return type methods that does not work with "-g" option
#CXX = clang++

# But everything works fine with GCC 4.9!
CXX = g++-4.9
#CXX = clang++-3.6

# Some hack to force the compiler from the test infrastructure:
ifdef FORCE_CXX
  CXX=$(FORCE_CXX)
endif

all: $(TARGETS)

run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), echo; echo $(command):; ./$(command); )

# Useful before a check...
clean:
	$(RM) $(TARGETS)

check:
	# Launch testing with lit tool from LLVM in current directory
	echo Using $(CXX) compiler:
	# lit can be found for example on Debian/Ubuntu in package
	# llvm-3.6-tools in /usr/lib/llvm-3.6/build/utils/lit/lit.py
	# so try before running the check:
	# export TRISYCL_LIT=/usr/lib/llvm-3.6/build/utils/lit/lit.py
	# The config file for triSYCL needs at least Python 3.3
	test "unset$$TRISYCL_LIT == unset" && echo 'Initialize TRISYCL_LIT variable to the path of "lit" command' ; echo 1
	python3 $$TRISYCL_LIT $(LITFLAGS) $(CHECKING_DIR)

check-compilers:
	# Launch the check with various compilers
	-FORCE_CXX=g++-4.9 $(MAKE) clean check
	-FORCE_CXX=clang++-3.6 $(MAKE) clean check
	-FORCE_CXX=g++ $(MAKE) clean check

# A special target to be called as from the test as "make execute
# TARGET=%s" with the right Makefile. There is a short-cut in the lit.cfg
# to use "RUN: %{execute}%s | %{filecheck} %s" in test files instead.

# Add a dependency on the binary name, i.e. without the extension
execute: $(basename $(TARGET))
	# Execute the compiled binary
	$<
