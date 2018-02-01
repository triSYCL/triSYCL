===============================================
 Architecture of triSYCL runtime and compiler
===============================================

.. section-numbering::

.. contents::

.. highlight:: Bash


triSYCL is a pure C++17 header runtime library to provide SYCL support
on CPU and accelerators.

To target devices, a device compiler based on Clang/LLVM is required
to extract the kernel from the SYCL program, compile it to the kernel
and generate on the host side some glue to call the kernel.

The main library is defined in ``include/CL/sycl.hpp`` as defined by the
SYCL standard.


Coding rules
============

All the headers are mainly called in alphabetic order in
``include/CL/sycl.hpp``.

More generally all the header files are included in alphabetic order,
but with the standard headers first, then `Boost` libraries and at
last the triSYCL headers.

The coding style is similar to the ``STL`` or ``Boost`` libraries,
with lines with less than 80 characters, to fit on a standard punch
card. :-)

To avoid ODR conflicts with a pure header library, C++17 ``inline`` is
used massively.


Basic organization
------------------

Each SYCL concept (for example a ``cl::sycl::queue``) is defined in its
own header file (for example ``include/CL/sycl/queue.hpp``) so it is
easy to find the definition of a concept.

If a concept requires more implementation details, these are detailed
inside a related ``detail`` directory, for example for the ``queue``,
besides the file ``include/CL/sycl/queue.hpp``, there might be also
inside ``include/CL/sycl/queue/detail`` the files

- ``queue.hpp`` defines an implementation detail
  common to the concept of ``queue`` with a virtual interface;

- ``host_queue.hpp`` inherits from the ``detail::queue`` above to
  implement the host device on CPU;

- ``opencl_queue.hpp`` inherits from the ``detail::queue`` above to
  implement an OpenCL device.

In ``include/CL/sycl/detail`` there are also some code used by the
implementation but not directly related to some SYCL classes.


Source-code documentation
-------------------------

The documentation of the source-code itself is based on Doxygen.

Doxygen modules are used to group the code elements according to
various themes.

To build the Doxygen documentation, in the top directory, run

.. code:: bash

  make

that will produce ``tmp/Doxygen/SYCL`` with the API documentation and
``tmp/Doxygen/triSYCL`` with the documented triSYCL implementation
source code.

To publish the documentation on GitHub:

.. code:: bash

  make publish

and finish as explained by the ``make`` output.


triSYCL architecture for CPU
============================

The implementation for CPU is a pure C++17 templated header library
and does not require a specific C++ or SYCL compiler.

The dataflow SYCL infrastructure between kernels related by
buffer/accessors dependencies is implemented in
`<../include/CL/sycl/command_group/detail/task.hpp>`_ with plain C++
``std::thread`` and ``std::condition_variable``. It should be updated
to a more efficient library in the future, such as TBB;

All the kernel code is accelerated with OpenMP, with various options
according to some macros parameters. Only the first dimension of
``range`` is parallelized with OpenMP in
`<../include/CL/sycl/parallelism/detail/parallelism.hpp>`_

Since in SYCL barriers are available and the CPU triSYCL
implementation does not use a compiler to restructure the kernel code,
it is implemented in SYCL with CPU threads provided by OpenMP. This is
massively inefficient.

Anyway, low-level OpenCL-style barriers should not be used in modern
SYCL code. Hierarchical parallelism, which is performance portable
between device and CPU, is preferable.

Otherwise, using an OpenCL target on CPU can be used to rely on the
CPU OpenCL stack to do CPU-friendly SIMD-ization of the
barrier-spaghetti code. But this relies on the triSYCL device
compiler...



triSYCL architecture for accelerator
====================================

When targeting an accelerator, even if SYCL is a pure C++ DSEL, a
specific compiler is required to extract the kernel code and compile
it to some target device and at the same time to compile on the host side
some glue code around the extraction boundary to transfer data to and
from the device and call the kernel itself.

The device compiler is based on Clang/LLVM 3.9 for now.

Since it is quite more experimental than the CPU path, it is not yet
merged into the main branches:

- device branch of triSYCL:
  https://github.com/triSYCL/triSYCL/tree/device

- Clang supporting triSYCL:
  https://github.com/triSYCL/clang/tree/sycl/release_39/master

- LLVM supporting triSYCL:
  https://github.com/triSYCL/llvm/tree/sycl/release_39/master


Installation & compilation
--------------------------

First download or clone the device compiler repositories, for example
with::

  git clone --branch sycl/release_39/master git@github.com:triSYCL/llvm.git
  cd llvm/tools
  git clone --branch sycl/release_39/master git@github.com:triSYCL/clang.git
  cd ../..

Then compile for example with::

  mkdir build
  cd build
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
    -DLLVM_ENABLE_CXX1Y=On \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DLLVM_BUILD_LLVM_DYLIB:BOOL=ON \
    -DLLVM_LINK_LLVM_DYLIB:BOOL=ON \
    ../llvm
  # Use -j8 to speed up compilation if you have 8 cores for example
  make -j8

You might replace the ``Release`` by ``Debug`` above if you want to
debug the compiler itself. Look at https://llvm.org/docs/CMake.html
for more information.

Compilation and installation of the triSYCL runtime::

  git clone --branch device git@github.com:triSYCL/triSYCL.git
  # Compile the triSYCL_tool command
  cd triSYCL/src
  make


Usage
-----

Unfortunately there is no driver yet to generate directly the host and
device part and it is up to the end-user for now, since it is still
experimental and in development. So using the compiler
is... painful. :-(

It is expected to be used as for example with examples from
`<../tests/device_compiler>`_. Everything is done from
`<../tests/Makefile>`_ when making a target ending with the
``.kernel_caller`` extension such as
``tests/device_compiler/single_task_vector_add_drt.kernel_caller``.

triSYCL assumes some recent Clang/LLVM installed, independently from
the one used by device compiler which might not be new enough.

A recent version of Boost is required. It is available with package
``libboost-all-dev`` on Debian/Ubuntu or with some more modern
specific versions such as ``libboost1.63-all-dev``.


Using OpenCL PoCL on CPU
~~~~~~~~~~~~~~~~~~~~~~~~

The device compiler generates the kernels as SPIR-df (*de-facto*),
which is SPIR 2.0 encoded with LLVM IR of a more recent version than
LLVM 3.4 expected by the SPIR specification. So a very modern SPIR
consumer is required, such as a recent PoCL. It is not the version
available in Ubuntu 17.10 for example, so you might compile and
install PoCL on your own...

Set up the environment::

  export LLVM_BUILD_DIR=<directory_where_LLVM_is_built>
  # Use PoCL OpenCL stack
  export BOOST_COMPUTE_DEFAULT_PLATFORM='Portable Computing Language'
  # Do not use another OpenCL stack if the one requested is not available
  export BOOST_COMPUTE_DEFAULT_ENFORCE=1

Compile and execute a small example::

  cd tests
  make -j2 device_compiler/single_task_vector_add_drt.kernel_caller
  device_compiler/single_task_vector_add_drt.kernel_caller
    [...]
    Queue waiting for kernel completion

    **** no errors detected


Using Xilinx SDx xocc for FPGA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let's assume you have installed Xilinx SDx somewhere. Initialize the
environment with something like::

  export XILINX_SDX=/opt/Xilinx/SDx/2017.2
  PATH=$PATH:$XILINX_SDX/bin

  export LLVM_BUILD_DIR=<directory_where_LLVM_is_built>
  # Use the Xilinx OpenCL stack
  export BOOST_COMPUTE_DEFAULT_PLATFORM=Xilinx
  # Do not use another OpenCL stack if the one requested is not available
  export BOOST_COMPUTE_DEFAULT_ENFORCE=1

Compile and execute a small example::

  cd tests
  make -j2 device_compiler/single_task_vector_add_drt.kernel_caller
  device_compiler/single_task_vector_add_drt.kernel_caller
    [...]
    Queue waiting for kernel completion

    **** no errors detected

Note that since the final code contains the FPGA bit-stream
configuration file and not the SPIR representation, it takes quite a
lot of time to be generated through SDx...


Testing infrastructure
======================

Look at `<testing.rst>`_ and `<../tests/README.rst>`_


Continuous integration
======================

Travis CI is used to validate triSYCL with its test suite from `tests/
<../tests>`_ on CPU and OpenCL with interoperability mode, using CMake
``ctest``.

The device compiler is not tested yet through Travis CI.

Look at `<../.travis.yml>`_ and `<../Dockerfile>`_ for the
configuration.
