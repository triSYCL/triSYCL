===============================================
 Architecture of triSYCL runtime and compiler
===============================================

.. section-numbering::

.. contents::

.. highlight:: C++


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

The implementation for CPU is a pure C++17 templated header library.

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


Testing infrastructure
======================

Look at `<testing.rst>`_ and `<../tests/README.rst>`_


Continuous integration
======================

Travis CI is used to validate triSYCL with its test suite from `tests/
<../tests>`_ on CPU and OpenCL with interoperability mode, using CMake
``ctest``.

The device compiler is not tested yet through Travis CI.

Look at ``.travis.yml`` and ``Dockerfile`` for the configuration.
