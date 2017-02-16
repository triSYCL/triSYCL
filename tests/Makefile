# Typical variables to tweak for the local environment.

# See doc/macros.rst for a description of triSYCL-specific macros.

# Set default compiler instead of default CXX=g++ value
# Assume Clang 3.9 at least
#CXX = clang++-3.9
# But everything works fine with GCC 5.4 at least
CXX = g++

# Set OpenCL_INCPATH variable to where CL/cl.h like headers are if
# required

# Set OpenCL_LIBPATH variable to where OpenCL library files are

# Set BOOST_COMPUTE_INCPATH variable to where a specific version of
# Boost.Compute is if required

# To use OpenMP to execute SYCL kernels:
CXXFLAGS += -fopenmp
# On Ubuntu, Clang has some issues to find omp.h, so use:
#CXXFLAGS += -I/usr/lib/gcc/x86_64-linux-gnu/6/include

## To disable asynchronous kernels, which is the default in SYCL
#CXXFLAGS += -DTRISYCL_NO_ASYNC

# For asynchronous kernels when OpenMP is not enabled
#LDLIBS += -lpthread

# To use OpenCL, set the TRISYCL_OPENCL flag
CXXFLAGS += -DTRISYCL_OPENCL
LDLIBS += -lOpenCL

# To enable debug and tracing:
CXXFLAGS += -g
#CXXFLAGS += -g -DTRISYCL_TRACE_KERNEL -DBOOST_LOG_DYN_LINK
#CXXFLAGS += -g -DTRISYCL_DEBUG -DBOOST_LOG_DYN_LINK
#CXXFLAGS += -g -DTRISYCL_DEBUG -DTRISYCL_TRACE_KERNEL -DTRISYCL_DEBUG_STRUCTORS -DBOOST_LOG_DYN_LINK
# To have DEBUG mode working
#LDLIBS += -lboost_log -lpthread
# To use Clang sanitizer modes
#CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
#CXXFLAGS += -fsanitize=thread -fno-omit-frame-pointer
#CXXFLAGS += -fsanitize=undefined -fno-omit-frame-pointer

# When XILINX_SDX environment variable is set, use the Xilinx SDx
# OpenCL compiler with these flags:
# Adapt to the real platform
XOCCFLAGS=--platform xilinx:adm-pcie-7v3:1ddr:3.0



# The rest is the Makefile boiling plate and should not be changed normally


# Compute the absolute directory name from the location of this Makefile
# so that we can compile from anywhere even if we use make -f
# <this_makefile> ... as used by make check:
triSYCL_DIR=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Use all the .cpp C++ files from the subdirectories that have a 1-1
# mapping with the binaries to generate, that means the binary is made
# from only one compilation unit (1 source file)
TARGETS = $(basename $(wildcard */*.cpp))

# There are also .cc C++ files that are used to test binaries made from
# multiple compilation units. There should be specific dependencies added
# later to deal with other compilation units to be linked to the .cc main
# executable. To avoid messing up with LLVM/LIT, the other compilation
# units have .C extensions
TARGETS_CC = $(basename $(wildcard */*.cc))
TARGETS += $(TARGETS_CC)

# Since the .C files generates some .o, think to remove them
FILES_C = $(wildcard */*.C)
CLEANING_TARGETS = $(TARGETS) $(FILES_C:%.C=%.o)

# The implementation uses C++14
CXXFLAGS += -Wall -std=c++1y -I$(triSYCL_DIR)/include \
	-I$(triSYCL_DIR)/tests/common

# Specify where OpenCL includes files are with OpenCL_INCPATH
ifdef OpenCL_INCPATH
	CXXFLAGS += -I$(OpenCL_INCPATH)
endif

# Specify where Boost.Compute is with BOOST_COMPUTE_INCPATH
ifdef BOOST_COMPUTE_INCPATH
	CXXFLAGS += -I$(BOOST_COMPUTE_INCPATH)
endif

# Specific flags for Boost.Compute
CXXFLAGS += -DBOOST_COMPUTE_DEBUG_KERNEL_COMPILATION \
	-DBOOST_COMPUTE_HAVE_THREAD_LOCAL \
	-DBOOST_COMPUTE_THREAD_SAFE

# Specify where OpenCL library files are with OpenCL_LIBPATH
ifdef OpenCL_LIBPATH
  LDFLAGS += -L$(OpenCL_LIBPATH)
endif

# By default run lit checking in the current directory
CHECKING_DIR ?= .

# Some hack to force the compiler from the test infrastructure with
# reentrant make:
ifdef FORCE_CXX
  CXX=$(FORCE_CXX)
endif

all: $(TARGETS)

run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), echo; echo $(command):; ./$(command); )

# Useful before a check...
clean:
	$(RM) $(CLEANING_TARGETS)

# Add some explicit targets
multiple_compilation_units/parallel_for: multiple_compilation_units/parallel_for_other.o

ifdef XILINX_SDX
# Compile explicit OpenCL kernels for Xilinx FPGA.
# Note this variable enables also the Lit tests guarded
# by "REQUIRES: xilinx-xocc"

# Use "order-only" prerequisites for kernels since if the kernel are
# changed, it does not imply to recompile the host part
SDAccel/opencl_kernel_vector_add_args: | SDAccel/vector_add.xclbin

# -lpthread is required by Xilinx OpenCL implementation
LDLIBS += -pthread

CLEANING_TARGETS += $(wildcard */*.xclbin)

# How to compile OpenCL kernel to Xilinx FPGA
%.xclbin: %.cl
	xocc $(XOCCFLAGS) --target hw -o $@ $<
endif

# Force recompilation of $(TARGETS_CC) binaries through the Makefile since
# LIT does not know the dependencies
check: $(TARGETS_CC)
	# Launch testing with lit tool from LLVM in current directory
	echo Using $(CXX) compiler:
	# lit can be found for example on Debian/Ubuntu in package
	# llvm-3.7-tools in /usr/lib/llvm-3.7/build/utils/lit/lit.py
	# so try before running the check:
	# export TRISYCL_LIT=/usr/lib/llvm-3.7/build/utils/lit/lit.py
	# The config file for triSYCL needs at least Python 3.3
	test "unset$$TRISYCL_LIT == unset" \
	  && echo 'Initialize TRISYCL_LIT variable to the path of "lit" command' ; \
	  echo 1
	python3 $$TRISYCL_LIT $(LITFLAGS) $(CHECKING_DIR)

check-compilers:
	# Launch the check with various compilers
	-FORCE_CXX=g++-5 $(MAKE) clean check
	-FORCE_CXX=clang++-3.9 $(MAKE) clean check
	-FORCE_CXX=g++ $(MAKE) clean check

# A special target to be called as from the test as "make execute
# TARGET=%s" with the right Makefile. There is a short-cut in the lit.cfg
# to use "RUN: %{execute}%s | %{filecheck} %s" in test files instead.

# Add a dependency on the binary name, i.e. without the extension
execute: $(basename $(TARGET))
	# Execute the compiled binary
	$<

# To verify everything is self-contained, run the target in a clone of the
# current branch of the current repository.
#
# Forward any clone-T target into a clone and make T in it
#
# Use for example:
# make clone-check
clone-%:
	rm -rf test-clone
	git clone --branch `git rev-parse --abbrev-ref HEAD` .. test-clone
	cd test-clone/tests; $(MAKE) $*
# To verify everything is self-contained, run the target in a new
# work-tree of the current branch of the current repository.
#
# Forward any clone-T target into a clone and make T in it
#
# Use for example:
# make clone-check
clone-%:
	# Remove some old tests
	rm -rf test-clone
	# The is a current issue when we create a lot of work-tree
	# at the same place because they are not pruned if the directory exist.
	#  So prune the old ones if any when the directory does not exist anymore
	git worktree prune
	# Create a work-tree from the same SHA-1 commit.
	# Use a work-tree instead of clone so we can test directly
	# from some remote commit.
	# Use a detached work-tree so we do not need a branch
	# and do not block an existing one
	git worktree add --detach test-clone `git rev-parse HEAD`
	cd test-clone/tests; $(MAKE) $*
