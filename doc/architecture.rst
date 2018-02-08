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

The main library is defined in `<../include/CL/sycl.hpp>`_ as defined by the
SYCL standard.


Coding rules
============

All the headers are mainly called in alphabetic order in
`<../include/CL/sycl.hpp>`_.

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
own header file (for example `<../include/CL/sycl/queue.hpp>`_) so it is
easy to find the definition of a concept.

If a concept requires more implementation details, these are detailed
inside a related ``detail`` directory, for example for the ``queue``,
besides the file `<../include/CL/sycl/queue.hpp>`_, there might be also
inside `<../include/CL/sycl/queue/detail>`_ the files

- `<../include/CL/sycl/queue/detail/queue.hpp>`_ defines an
  implementation detail common to the concept of ``queue`` with a
  virtual interface;

- `<../include/CL/sycl/queue/detail/host_queue.hpp>`_ inherits from
  the ``detail::queue`` above to implement the host device on CPU;

- `<../include/CL/sycl/queue/detail/opencl_queue.hpp>`_ inherits from
  the ``detail::queue`` above to implement an OpenCL device.

In `<../include/CL/sycl/detail>`_ there are also some code used by the
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


High-level workflow
-------------------

.. figure:: images/2018-01-22--26-triSYCL-workflow-SPIR.svg
   :width: 100%
   :alt: High-level compilation workflow in triSYCL
   :name: Figure 1

   Figure 1: High-level view of the compilation workflow in triSYCL.

When compiling on CPU, since triSYCL relies on the fact that SYCL is a
pure C++ executable DSEL, the C++ SYCL code is just compiled with any
host compiler (top of `Figure 1`_) which includes the SYCL runtime
(bottom left of `Figure 1`_) which is a plain C++ header file. A CPU
executable is generated, using OpenMP for multithreading.

If some OpenCL features are used through the interoperability mode
(non-single-source SYCL), then an OpenCL library is required to
interact with some OpenCL devices.

When using SYCL in single-source mode on device, the compilation flow
is quite more complex because it requires a device compiler to split
and compile the code for the final target.

The Clang/LLVM-based device compiler (bottom of `Figure 1`_) compiles
the C++ SYCL code as for CPU only, but just keep the kernel part of
the code and produce a simple portable intermediate representation
(SPIR) of the kernels.  For now, since SPIR-V is not yet widely used,
triSYCL uses SPIR-df *de-facto*, a non-conforming SPIR 2.0 encoded in
something newer than LLVM 3.4 IR.

Then this SPIR-df output is optionally compiled by some vendor
compiler to speed-up the launch time by doing some compilation
ahead. With PoCL it is not done (dashed arrow line) but for FPGA it is
done ahead-of-time since compilation is very slow.

In single-source mode on device, the source code has also to go
through the device compiler, but to do the dual operation: to remove the
kernel code and just to keep the host code. This is also where some glue to
call the kernels and to do the argument serialization is done.

The kernel binary generated by the other compiler flow is also
included in the host code so that the main host executable is
self-contained and can start the kernel on the device without having
to load the binary from an external file.


Low-level view of the device compiler workflow
----------------------------------------------

.. figure:: images/triSYCL-device-compiler-workflow.svg
   :width: 100%
   :alt: Low-level compilation workflow in triSYCL
   :name: Figure 2

   Figure 2: Low-level view of the compilation workflow in triSYCL.

The real workflow is currently implemented in `<../tests/Makefile>`_
and this is the current source of truth. The path to go for example
from a ``ex.cpp`` file to a final ``ex.kernel_caller`` is summarized
on `Figure 2`_,

Each intermediate file is characterized by a specific extension:

``.cpp``
  for the single-source SYCL C++ input file;

``.bc``
  some LLVM IR bitcode;

``.ll``
  some LLVM IR in textual assembly syntax;

``.kernel_caller``
  for the final host executable, with the kernel binary internalized
  so the host can load and launch the kernels on the devices without
  external files.

Note that the file without any extension is actually the normal
CPU-only executable, which does not appear in this picture because it
is about compiling for device instead.

All the SYCL LLVM passes are in the ``lib/SYCL`` directory of LLVM.


Host-side compilation flow
~~~~~~~~~~~~~~~~~~~~~~~~~~

The file extensions used on the host side are:

``.pre_kernel_caller.ll``
  the SYCL C++ code compiled by Clang for the host side, including
  the call of the kernels;

``.kernel_caller.ll``
  the LLVM IR of the host code after the LLVM triSYCL pass
  transformations;

To generate the ``.pre_kernel_caller.ll`` file, the source code is
compiled with::

  clang -O3 -sycl

which is basically ``clang`` unchanged, but with ``loop-idiom``
detection pass skipped because otherwise it generates some memory copy
intrinsic functions that prevents some argument flattening to work
later.

The ``-O3`` is important to generate optimized minimal code that can
be massaged later, with a lot of in-lining to have the C++ constructs
to disappear. Otherwise less optimized code breaks a lot of
assumptions in the triSYCL-specific LLVM passes later.

The compilation flow to generate the final ``.kernel_caller.ll`` file
is based on LLVM ``opt`` to apply a sequence of LLVM passes:

``-globalopt -deadargelim``
  to clean-up the code before SYCL massaging;

``-SYCL-args-flattening``
  is a fundamental SYCL-specific pass that takes the lambda capture
  (basically a C++ structure passed by address) of a SYCL kernel
  lambda expression and flatten it as its content. So basically if the
  capture has several scalar and accessor parameters, the structure
  address used in the function call is replaced by a function call
  with all the parameters explicitly passed as argument. This makes
  the classical OpenCL-style kernel parameter to show up;

``-loop-idiom -deadargelim``
  then the loop-idiom detection pass which was not applied before to
  avoid choking the ``SYCL-args-flattening`` pass can now be applied
  to optimize some loops and generate the LLVM intrinsics representing
  memory copies and initialization for example;

``-deadargelim``
  removes some dead code that might be left by previous passes;

``-SYCL-serialize-arguments``
  is another fundamental SYCL-specific pass on host side which
  replaces a kernel function call by some calls to the runtime to
  select the kernel and serialize all the kernel arguments.

  The input code from the triSYCL headers of the form

  .. code-block:: C++

     cl::sycl::detail::set_kernel_task_marker(t);
     cl::sycl::detail::instantiate_kernel<KernelName>(/* flatten args */);

  is replaced by

  .. code-block:: C++

     cl::sycl::drt::set_kernel(detail::task &task, const char *kernel_name,
                               const char *kernel_short_name);
     // For each parameter call:
     // either for a scalar argument
     cl::sycl::drt::serialize_arg(detail::task &task, std::size_t index,
                                  void *arg, std::size_t arg_size);
     // or for an accessor argument
     cl::sycl::drt::serialize_accessor_arg(detail::task &task, std::size_t index,
                                           void *arg, std::size_t arg_size)

  The marking functions generated by triSYCL headers are in
  `<../include/CL/sycl/detail/instantiate_kernel.hpp>`_ while the
  functions used by the transformed code are in
  `<../include/CL/sycl/device_runtime.hpp>`_. The functions from
  ``cl::sycl::drt::`` are the link to the underlying runtime, such as
  OpenCL.

``-deadargelim``
  again to removes some dead code that might be left by previous pass.


Device-side compilation flow
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The file extensions used on the kernel side are:

``.pre_kernel.ll``
  the SYCL C++ code compiled by Clang for the host side, including
  the call of the kernels;

``.kernel.ll``
  the LLVM IR of the host code after the LLVM triSYCL pass
  transformations;

``.kernel.bin``
  is for the kernel binary to be shipped into the final host
  executable. This is typically a SPIR LLVM IR bitcode or an FPGA
  bitstream configuration;

``.kernel.internalized.cxx``
  is the kernel binary represented as C++ code so it can just be
  compiled by a C++ compiler to have it internalized into the final
  host binary and used by the runtime.

  It is constructed from the ``.kernel.bin`` file through the helper
  ``triSYCL_tool --source-in``.

To generate the ``.pre_kernel.ll`` file, the source code is compiled
with::

  clang -O3 -DTRISYCL_DEVICE -sycl -sycl-is-device

This is similar to the compilation for the host side and the ``-O3``
is important for the same reasons. ``-DTRISYCL_DEVICE`` is used so
the triSYCL headers behave slightly differently on the device code,
mainly enabling some address-space related code used to represent
OpenCL ``global`` or ``local`` memory for example.

Like for the host side path, the compilation flow to generate the
final ``.kernel.ll`` file is based on LLVM ``opt`` to apply a sequence
of LLVM passes. While :

``-globalopt -deadargelim -SYCL-args-flattening -deadargelim``
  are applied as for the host side. It is important to have globally
  the same code compiled with the same passes for both host and device
  side to keep the code synchronized before serialization. Otherwise
  it would lead to some mismatch and some wrong global code at the
  end;

``-SYCL-kernel-filter``
  this is one of the most important SYCL-specific pass on the device
  side, to extract the kernels from the single-source code. Actually
  it works in 2 passes, in a mark-and-sweep approach. Here is the
  first pass that marks all the kernel with external linkage (tricking
  the compiler as it might be useful from outside) and all the
  non-kernel part with internal linkage;

``-globaldce``
  this is the second stage of kernel selection. It will remove all the
  dead code of the program. Since only the kernels have been marked as
  potentially used from the outside, after application of this pass,
  only what is transitively useful for the kernels are left. So only
  remains the device code;

``-RELGCD``
  compiling C++ comes with an ABI storing the lists of global static
  constructors and destructors. Unfortunately even if at the end these
  lists are empty because of SYCL specification, they are not removed
  by ``-globaldce`` and it is not supported by SPIR yet. So this
  SYCL-specific pass removes the empty list of global constructors or
  destructors (RELGCD);

``-reqd-workgroup-size-1``
  in the case the kernel are compiled with only 1 SPIR work-group with
  1 work-item (common use case on FPGA), this SYCL-specific pass add a
  SPIR metadata on the kernels to specify it will be called with only
  1 work-item. This way the target compiler can spare some resources
  on the device;

``-inSPIRation``
  is the SYCL-specific pass generating the SPIR 2.0-style LLVM IR
  output. Since it generates LLVM IR with the version of the recent
  LLVM used, it is quite more modern that the official SPIR 2.0 based
  on LLVM 3.4 IR. So it is a SPIR "de-facto", which is nevertheless
  accepted by some tools. But by using a down-caster, it could
  probably make some decent official SPIR 2.0 encoded in LLVM 3.4
  IR). Or using a SPIR-V back-end could generate some SPIR-V code.


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
