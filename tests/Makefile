# Typical variables to tweak for the local environment.

# See doc/macros.rst for a description of triSYCL-specific macros.

# Set default compiler instead of default CXX=g++ value
# Assume Clang 3.9 at least
CXX = clang++
# But everything works fine with GCC 5.4 at least
#CXX = g++

# The device compiler to use
CL_CXX = clang++-3.9

# Set OpenCL_INCPATH variable to where CL/cl.h like headers are if
# required

# Set OpenCL_LIBPATH variable to where OpenCL library files are

# Set BOOST_COMPUTE_INCPATH variable to where a specific version of
# Boost.Compute is if required

# To use OpenMP to execute SYCL kernels:
#CXXFLAGS += -fopenmp
# On Ubuntu, Clang has some issues to find omp.h, so use:
#CXXFLAGS += -I/usr/lib/gcc/x86_64-linux-gnu/6/include

## To disable asynchronous kernels, which is the default in SYCL
#CXXFLAGS += -DTRISYCL_NO_ASYNC

# For asynchronous kernels when OpenMP is not enabled
LDLIBS += -lpthread

# To use OpenCL, set the TRISYCL_OPENCL flag
CXXFLAGS += -DTRISYCL_OPENCL
# To use generic OpenCL:
LDLIBS += -lOpenCL
# To use PoCL directly instead of generic OpenCL:
#LDFLAGS += $(shell pkg-config --libs-only-L --libs-only-other pocl)
#LDLIBS += $(shell pkg-config --libs-only-l pocl)
#CXXFLAGS += $(shell pkg-config --cflags pocl)

# To enable debug and tracing:
#CXXFLAGS += -g
# With optimization and without assertions
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -DNDEBUG
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
triSYCL_DIR = $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

# Use all the .cpp C++ files from the subdirectories that have a 1-1
# mapping with the binaries to generate, that means the binary is made
# from only one compilation unit (1 source file)
FILES_CPP = $(wildcard */*.cpp)
TARGETS = $(basename $(FILES_CPP))

# There are also .cc C++ files that are used to test binaries made from
# multiple compilation units. There should be specific dependencies added
# later to deal with other compilation units to be linked to the .cc main
# executable. To avoid messing up with LLVM/LIT, the other compilation
# units have .C extensions
FILES_CC = $(wildcard */*.cc)
TARGETS_CC = $(basename $(FILES_CC))
TARGETS += $(TARGETS_CC)

# Since the .C files generates some .o, think to remove them
FILES_C = $(wildcard */*.C)
CLEANING_TARGETS = $(TARGETS) $(FILES_C:%.C=%.o)

# The kernel files
KERNELS = $(FILES_CC:%.cc=%.kernel.ll) \
	$(FILES_C:%.C=%.kernel.ll) \
	$(FILES_CPP:%.cpp=%.kernel.ll)

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

# Compile all the executables
all: $(TARGETS)

# Compile all the kernel bitcodes
kernels: $(KERNELS)

# Run all the tests, once compiled
run: $(TARGETS)
	# Execute each element of TARGETS
	$(foreach command, $(TARGETS), echo; echo $(command):; ./$(command); )

# Useful before a check...
clean:
	$(RM) $(CLEANING_TARGETS)

# Add some explicit targets
multiple_compilation_units/parallel_for: multiple_compilation_units/parallel_for_other.o

# Compile some kernels to SPIR as an "order-only" prerequisite:
SPIR/opencl_kernel_output_42: | SPIR/output_42.spir64
SPIR/opencl_kernel_vector_add_args: | SPIR/vector_add.spir64
SPIR/opencl_kernel_vector_add_int: | SPIR/vector_add_int.spir64
SPIR/single_task_vector_increment_drt: | SPIR/add_42.spir64

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
	# llvm-3.9-tools in /usr/lib/llvm-3.9/build/utils/lit/lit.py
	# so try before running the check:
	# export TRISYCL_LIT=/usr/lib/llvm-3.9/build/utils/lit/lit.py
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

# Feed the test into Clang-based RTags indexer to be used inside Emacs IDE
# with make some_cpp_file.rtags
%.rtags: %.cpp
	$(MAKE) --just-print --assume-new=$< $* | rc -c -

# To debug the Clang/LLVM triSYCL compiler
# Assume that CXX contains some Clang compiler version

# Keep intermediate file for debuging purpose
.PRECIOUS: %.bc %.ll %.spir64 %.kernel.spir.cpp \
	%.pre_kernel.bc %.pre_kernel.ll %.pre_kernel_caller.ll \
	%.kernel.bc %.kernel.ll

# The LLVM bitcode
%.bc: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -c -o $@ $<
%.bc: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -c -o $@ $<
%.bc: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -c -o $@ $<

# The LLVM assembly code
%.ll: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -S -o $@ $<
%.ll: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -S -o $@ $<
%.ll: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -S -o $@ $<

# The LLVM assembly code for the code expected to call the kernels
%.pre_kernel_caller.ll: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -sycl \
	  -emit-llvm -S -o $@ $<
%.pre_kernel_caller.ll: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -sycl \
	  -emit-llvm -S -o $@ $<
%.pre_kernel_caller.ll: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -sycl \
	  -emit-llvm -S -o $@ $<

# The LLVM assembly code for the code before kernels
%.pre_kernel.ll: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  -emit-llvm -S -o $@ $<
%.pre_kernel.ll: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  -emit-llvm -S -o $@ $<
%.pre_kernel.ll: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  -emit-llvm -S -o $@ $<

# How to compile OpenCL kernel to 64-bit SPIR 2.0
%.spir64: %.cl
	# Assume here that CXX is actually a kind of clang++
	$(CL_CXX) -cc1 -emit-llvm-bc -triple spir64-unknown-unknown \
	  -include $(triSYCL_DIR)/include/CL/sycl/detail/SPIR/opencl_spir.h -o $@ $<

# Process bitcode with SYCL pass to generate kernels
%.kernel.bc: %.pre_kernel.ll
	#opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so -debug -stats \
	#  -globalopt -deadargelim -SYCL-args-flattening -deadargelim \
	#  -SYCL-kernel-filter \
	#  -globaldce -o $@ $<
	#opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so -stats -SYCL-kernel-filter \
	#  -debug -print-after-all -o $@ $<
	#opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so -debug -stats -SYCL-kernel-filter \
	#  -o $@ $<
	#opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so -stats  -debug \
	#  -SYCL-annotation -SYCL-kernel-filter -o $@ $<
	$(LLVM_BUILD_DIR)/bin/opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so -debug -stats \
	  -globalopt -deadargelim -argpromotion -deadargelim \
	  -SYCL-kernel-filter -globaldce -RELGCD -inSPIRation \
	  -o $@ $<

# Internalize the bitcode into cl::sycl::drt::code::program with a C++ file
%.kernel.spir.cpp: %.kernel.bc
	triSYCL_tool --source-in $< --output $@

# Process bitcode with SYCL pass to generate kernels
%.kernel_caller.bc: %.pre_kernel_caller.ll
	$(LLVM_BUILD_DIR)/bin/opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so -debug -stats \
	  -globalopt -deadargelim -SYCL-args-flattening \
	  -SYCL-serialize-arguments -deadargelim -o $@ $<

# Construct the object file of the kernel caller from the target assembly
# Rely on the default Make built-in rules for now
#%.kernel_caller.o: %.kernel_caller.s

# Generate optimized target machine assembly code from LLVM IR
%.kernel_caller.s: %.kernel_caller.ll
	llc -O=3 -o $@ $<

# Link the C++-generated kernel caller to the final program
%.kernel_caller: %.kernel_caller.ll %.kernel.spir.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

# Generate LLVM assembly code from bit-code
%.ll: %.bc
	llvm-dis -o $@ $<

# Generate LLVM assembly code from SPIR bit-code
%.spir64.ll: %.spir64
	llvm-dis -o $@ $<

# A pretty-printed C++ view of the Clang AST
%.ast-c++: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -Xclang -ast-print -c -o $@ $<
%.ast-c++: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -Xclang -ast-print -c -o $@ $<
%.ast-c++: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -Xclang -ast-print -c -o $@ $<

# The Clang AST
%.ast: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -Xclang -ast-dump \
	  -fno-color-diagnostics -fsyntax-only $< > $@
%.ast: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -Xclang -ast-dump \
	  --fno-color-diagnostics -fsyntax-only $< > $@
%.ast: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -Xclang -ast-dump \
	  --fno-color-diagnostics -fsyntax-only $< > $@


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
