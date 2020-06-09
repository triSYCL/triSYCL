# Typical variables to tweak for the local environment.

# See doc/macros.rst for a description of triSYCL-specific macros.

# Set default compiler instead of default CXX=g++ value
# Assume some recent compiler
CXX = clang++
# To use libc++ instead of libstdc++, from libc++-dev & libc++abi-dev packages
#CXXFLAGS += -stdlib=libc++

# But everything works fine with GCC 9 at least
#CXX = g++

# The device compiler to use.
# Assume that the device compiler is in $(LLVM_BUILD_DIR) and this
# environment variable is set.
CL_CXX = $(LLVM_BUILD_DIR)/bin/clang++

# Set OpenCL_INCPATH variable to where CL/cl.h like headers are if
# required

# Set OpenCL_LIBPATH variable to where OpenCL library files are

# Set BOOST_COMPUTE_INCPATH variable to where a specific version of
# Boost.Compute is if required

# To use OpenMP to execute SYCL kernels (Clang needs libomp-dev package)
#CXXFLAGS += -fopenmp

## To disable asynchronous kernels, which is the default in SYCL
#CXXFLAGS += -DTRISYCL_NO_ASYNC

# For asynchronous kernels when OpenMP is not enabled
LDLIBS += -lpthread

# To use OpenCL, set the TRISYCL_OPENCL flag
CXXFLAGS += -DTRISYCL_OPENCL
# To use generic OpenCL:
LDLIBS += -lOpenCL
#To use Xilinx OpenCL library without ICD
#LDLIBS += -lxilinxopencl
# To use PoCL directly instead of generic OpenCL:
#LDFLAGS += $(shell pkg-config --libs-only-L --libs-only-other pocl)
#LDLIBS += $(shell pkg-config --libs-only-l pocl)
#CXXFLAGS += $(shell pkg-config --cflags pocl)

# To enable debug and tracing:
CXXFLAGS += -g
# With optimization and without assertions
#CXXFLAGS += -O3 -DNDEBUG -DTRISYCL_DEBUG -DBOOST_LOG_DYN_LINK
#CXXFLAGS += -DNDEBUG
#CXXFLAGS += -g -DTRISYCL_TRACE_KERNEL -DBOOST_LOG_DYN_LINK
#CXXFLAGS += -g -DTRISYCL_DEBUG -DBOOST_LOG_DYN_LINK
#CXXFLAGS += -g -DTRISYCL_DEBUG -DTRISYCL_TRACE_KERNEL -DTRISYCL_DEBUG_STRUCTORS -DBOOST_LOG_DYN_LINK
# To have DEBUG mode working
LDLIBS += -lboost_log -lpthread
# To have Boost.Fibers
LDLIBS += -lboost_fiber -lboost_context -lboost_thread -lboost_system

# To use Clang sanitizer modes
#CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
#CXXFLAGS += -fsanitize=thread -fno-omit-frame-pointer
#CXXFLAGS += -fsanitize=undefined -fno-omit-frame-pointer

# More flags for the kernel compilation flow:
# Counterbalance the effect of -O3 by avoiding vector types in SPIR output
# or inlining that might duplicate the code too early
CXXFLAGS_KERNEL = -fno-vectorize -fno-unroll-loops

ifdef TRISYCL_USE_OPENCL_ND_RANGE
  OPTIONAL_ND_RANGE = -DTRISYCL_USE_OPENCL_ND_RANGE
endif

# Specify a target device, otherwise it defaults to xilinx_vcu1525_dynamic_5_1
ifdef TARGET_DEVICE
  DEVICE_XOCC := $(TARGET_DEVICE)
else
  DEVICE_XOCC := xilinx_vcu1525_dynamic_5_1
endif

# Defaults to look for existing installed hardware, otherwise it will check for
# a defined emulation mode (XCL_EMULATION_MODE is also used by emconfigutil and
# the SDAccel examples)
ifdef XCL_EMULATION_MODE
  DEVICE_MODE := $(XCL_EMULATION_MODE)
else
  DEVICE_MODE := hw
endif

# When XILINX_SDX environment variable is set, use the Xilinx SDx OpenCL
# compiler with these XOCC specific flags. As well as add xilinx FPGA specific
# optimization passes.
ifdef XILINX_SDX
  XOCCFLAGS = --platform $(DEVICE_XOCC) --save-temps

  XILINX_SPECIFIC_PASS = -reqd-workgroup-size-1
endif
# xocc parameter to specify C++ version
XOCCCPPPARAM=--xp prop:kernel.vector_add.kernel_flags=-std=c++0x
# xocc parameter to work Clang/LLVM 3.9 flow
XOCC39PARAM=--xp param:compiler.version=3.9

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

# The kernel files, mainly defined here for debug or cleaning purpose:
# Kernel LLVM assembly code
KERNELS_LL = $(FILES_CC:%.cc=%.kernel.ll) \
	$(FILES_C:%.C=%.kernel.ll) \
	$(FILES_CPP:%.cpp=%.kernel.ll)
# Kernel LLVM assembly code
KERNELS_BC = $(KERNELS_LL:%.kernel.ll=%.kernel.bc)
# The kernel binary to be internalized into the host code, typically
# SPIR or FPGA bit-stream
KERNELS_BIN = $(KERNELS_LL:%.kernel.ll=%.kernel.bin)
# The kernel binary internalized into the host code as a C++ program
KERNELS_INTERNALIZED_CXX = $(KERNELS_LL:%.kernel.ll=%.kernel.internalized.cxx)
# The pre-kernel in LLVM assembly, directly output from Clang
PRE_KERNELS_LL = $(KERNELS_LL:%.kernel.ll=%.pre_kernel.ll)
# The pre-kernel caller in LLVM assembly, directly output from Clang
PRE_KERNELS_CALLER_LL = $(KERNELS_LL:%.kernel.ll=%.pre_kernel_caller.ll)
# The kernel caller in LLVM assembly
KERNELS_CALLER_LL = $(KERNELS_LL:%.kernel.ll=%.kernel_caller.ll)
# The kernel caller in LLVM bitcode
KERNELS_CALLER_BC = $(KERNELS_LL:%.kernel.ll=%.kernel_caller.bc)
# The final kernel caller executable
KERNELS_CALLER = $(KERNELS_LL:%.kernel.ll=%.kernel_caller)
# The xpirbc file used to generate the XO file
KERNELS_XPIRBC= $(KERNELS_LL:%.kernel.ll=%.xpirbc)
# The XO file used to generate the bin (xclbin) file
KERNELS_XO= $(KERNELS_LL:%.kernel.ll=%.kernel.xo)
# Info file that XOCC generates when compiling the xclbin file
KERNELS_BIN_INFO = $(KERNELS_LL:%.kernel.ll=%.kernel.bin.info)

CLEANING_TARGETS += $(KERNELS_BC) $(KERNELS_BIN) $(KERNELS_INTERNALIZED_CXX) \
	$(PRE_KERNELS_LL) $(PRE_KERNELS_CALLER_LL) $(KERNELS_CALLER_LL) \
	$(KERNELS_CALLER_BC) $(KERNELS_CALLER) $(KERNELS_XPIRBC) $(KERNELS_XO) \
	$(KERNELS_BIN_INFO)

# The implementation uses C++17.
# Use -Wno-ignored-attributes to avoid a lot of warning with
# Boost.Compute and g++ version >= 6
CXXFLAGS += -Wall -Wno-ignored-attributes -std=c++17 -I$(triSYCL_DIR)/include \
	-I$(triSYCL_DIR)/tests/common

# Vivado HLS path
ifdef XILINX_SDX
  CXXFLAGS += -I$(XILINX_SDX)/Vivado_HLS/include \
		-I$(XILINX_PATH)/Vivado_HLS/common/technology/autopilot
endif

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
kernels: $(KERNELS_LL)

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
SDAccel/hls_with_cppkernel_opencl_kernel_int_vector_add_args: | \
	SDAccel/hls_int_vector_add_kernel.xclbin
SDAccel/hls_with_cppkernel_opencl_kernel_ap_vector_add_args: | \
	SDAccel/hls_ap_vector_add_kernel.xclbin

# -lpthread is required by Xilinx OpenCL implementation
LDLIBS += -pthread

CLEANING_TARGETS += $(wildcard */*.xclbin)

# How to compile OpenCL kernel to Xilinx FPGA
%.xclbin: %.cl
	xocc $(XOCCFLAGS) --target $(DEVICE_MODE) -o $@ $<

# How to compile Vivado HLS C++ kernel to Xilinx FPGA
%.xclbin: %.cxx
	xocc --kernel vector_add --input_files $< $(XOCCCPPPARAM) $(XOCCFLAGS) --target $(DEVICE_MODE) -o $@
endif

# Force recompilation of $(TARGETS_CC) binaries through the Makefile since
# LIT does not know the dependencies
check:  $(TARGETS_CC)
	# Launch testing with lit tool from LLVM in current directory
	echo Using $(CXX) compiler:
	# lit can be found for example on Debian/Ubuntu in package
	# llvm-10-tools in /usr/lib/llvm-10/build/utils/lit/lit.py
	# so try before running the check:
	# export TRISYCL_LIT=/usr/lib/llvm-10/build/utils/lit/lit.py
	# The config file for triSYCL needs at least Python 3.3
	test "unset$$TRISYCL_LIT == unset" \
	  && echo 'Initialize TRISYCL_LIT variable to the path of "lit" command' ; \
	  echo 1
	python3 $$TRISYCL_LIT $(LITFLAGS) $(CHECKING_DIR)

# Display the list of LIT tests
check-list:
	python3 $$TRISYCL_LIT --show-tests $(LITFLAGS) $(CHECKING_DIR) | \
	  sed -e '1,/-- Available Tests --/d' -e 's/.\(cpp\|cc\)$$//' -e 's/  triSYCL :: //' | sort


check-compilers:
	# Launch the check with various compilers
	-FORCE_CXX=g++-9 $(MAKE) clean check
	-FORCE_CXX=clang++-9 $(MAKE) clean check
	-FORCE_CXX=g++ $(MAKE) clean check

# A special target to be called as from the test as "make execute
# TARGET=%s" with the right Makefile. There is a short-cut in the lit.cfg
# to use "RUN: %{execute}%s | %{filecheck} %s" in test files instead.

# Add a dependency on the binary name, i.e. without the extension
execute: $(basename $(TARGET))
	# Execute the compiled binary
	$<

# Feed the test into Clang-based RTags indexer to be used inside Emacs IDE
# with make some_cpp_file.rtags.
# Use intermediate "echo $(shell ...)" before sending to rtags to
# evaluate any ` `, such as the one with pkgconfig for gtkmm
%.rtags: %.cpp
	echo $(shell $(MAKE) --just-print --assume-new=$< $*) | rc -c -

# To debug the Clang/LLVM triSYCL compiler
# Assume that CXX contains some Clang compiler version

# Keep intermediate file for debuging purpose
.PRECIOUS: %.bc %.ll %.spir64 %.kernel.internalized.cxx \
  %.kernel_caller.bc %.kernel_caller.ll \
  %.pre_kernel.bc %.pre_kernel.ll \
  %.pre_kernel_caller.bc %.pre_kernel_caller.ll \
  %.pre_kernel.bc %.pre_kernel.ll %.pre_kernel_caller.ll \
  %.kernel.bc %.kernel.bin %.kernel.ll %.kernel.xo

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
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -sycl $(OPTIONAL_ND_RANGE) \
	  -emit-llvm -S -o $@ $<
%.pre_kernel_caller.ll: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -sycl $(OPTIONAL_ND_RANGE) \
	  -emit-llvm -S -o $@ $<
%.pre_kernel_caller.ll: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -sycl $(OPTIONAL_ND_RANGE) \
	  -emit-llvm -S -o $@ $<

# The LLVM assembly code for the code before kernels
%.pre_kernel.ll: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  $(OPTIONAL_ND_RANGE) $(CXXFLAGS_KERNEL) -emit-llvm -S -o $@ $<
%.pre_kernel.ll: %.C
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  $(OPTIONAL_ND_RANGE) $(CXXFLAGS_KERNEL) -emit-llvm -S -o $@ $<
%.pre_kernel.ll: %.cc
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  $(OPTIONAL_ND_RANGE) $(CXXFLAGS_KERNEL) -emit-llvm -S -o $@ $<

# The Clang AST of the pre-kernel code
%.pre_kernel.ast: %.cpp
	$(CL_CXX) $(CPPFLAGS) $(CXXFLAGS) -DTRISYCL_DEVICE -sycl -sycl-is-device \
	  $(CXXFLAGS_KERNEL) -Xclang -ast-dump \
	  -fsyntax-only $< > $@

# How to compile OpenCL kernel to 64-bit SPIR 2.0
%.spir64: %.cl
	# Assume here that CXX is actually a kind of clang++
	$(CL_CXX) -cc1 -emit-llvm-bc -triple spir64-unknown-unknown \
	  -include $(triSYCL_DIR)/include/CL/sycl/detail/SPIR/opencl_spir.h -o $@ $<

# Process bitcode with SYCL passes to generate kernels
%.kernel.bc: %.pre_kernel.ll
	# To debug, think about using options like -debug -stats -print-after-all...
	$(LLVM_BUILD_DIR)/bin/opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so \
	  -globalopt -deadargelim -SYCL-args-flattening -deadargelim \
	  -SYCL-kernel-filter -globaldce -RELGCD $(XILINX_SPECIFIC_PASS) \
	  -inSPIRation -globaldce -o $@ $<

#  If the target is a Xilinx FPGA kernel, use the Xilinx compiler with
#  the required extension for the SPIR input file
ifdef XILINX_SDX
%.kernel.xo: %.kernel.bc
	cp -p $< $*.xpirbc
	# TODO: deal with several kernels: -k TRISYCL_kernel_0 should be replaced
	# with -k all, but doesn't appear to work at the moment
	# Generate XO file from xpirbc
	xocc $(XOCCFLAGS) --target $(DEVICE_MODE) -c \
	  -k TRISYCL_kernel_0 -o $@ $*.xpirbc

	# Generate bin (xclbin) file from XO file (-l linking)
%.kernel.bin: %.kernel.xo
	xocc $(XOCCFLAGS) --target $(DEVICE_MODE) -l \
	  -k TRISYCL_kernel_0 -o $@ $*.kernel.xo
else
# If the target is SPIR, just use the bitcode as the kernel binary
%.kernel.bin: %.kernel.bc
	cp -p $< $@
endif

# Internalize the kernel binary into cl::sycl::drt::code::program with
# a C++ file
%.kernel.internalized.cxx: %.kernel.bin
	$(triSYCL_DIR)/src/triSYCL_tool --source-in $< --output $@

# Process bitcode with SYCL passes to generate kernel callers
%.kernel_caller.bc: %.pre_kernel_caller.ll
	# To debug, think about using options like -debug -stats -print-after-all...
	$(LLVM_BUILD_DIR)/bin/opt -load $(LLVM_BUILD_DIR)/lib/SYCL.so \
	  -globalopt -deadargelim -SYCL-args-flattening -loop-idiom -deadargelim \
	  -SYCL-serialize-arguments -deadargelim -o $@ $<

# Construct the object file of the kernel caller from the target assembly
# Rely on the default Make built-in rules for now
#%.kernel_caller.o: %.kernel_caller.s

# Generate optimized target machine assembly code from LLVM IR
%.kernel_caller.s: %.kernel_caller.ll
	llc -O=3 -o $@ $<

# Link the C++-generated kernel caller to the final program
# \todo Do not use .ll everywhere but .bc since it is not upward compatible
%.kernel_caller: %.kernel_caller.ll %.kernel.internalized.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

# Generate LLVM assembly code from bit-code
%.ll: %.bc
	$(LLVM_BUILD_DIR)/bin/llvm-dis -o $@ $<

# Generate LLVM assembly code from SPIR bit-code
%.spir64.ll: %.spir64
	$(LLVM_BUILD_DIR)/bin/llvm-dis -o $@ $<

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


# Test with cmake/ctest
ctest:
	# Think to delete this directory first if full recompilation is required...
	mkdir -p ../build
	cd ../build; cmake .. -DCMAKE_CXX_COMPILER=$(CXX) \
	   -DTRISYCL_OPENCL=ON; make -j`nproc`; ctest

# Only display the test list
ctest-list:
	# Think to delete this directory first if full recompilation is required...
	mkdir -p ../build
	cd ../build; cmake .. -DTRISYCL_OPENCL=ON; \
	  ctest --show-only | sed -e '1d' -e '/^$$/,$$d' -e 's/.*: //' | sort
