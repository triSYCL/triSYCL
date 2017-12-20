triSYCL
+++++++

.. include:: doc/common-includes.rst

.. image:: https://travis-ci.org/triSYCL/triSYCL.svg?branch=master
    :target: https://travis-ci.org/triSYCL/triSYCL

Introduction
------------

triSYCL_ is an open source implementation to experiment with
the specification of the OpenCL_ SYCL_ 1.2.1 and 2.2 `C++`_ layer and
to give feedback to the Khronos_ OpenCL_ SYCL_ and OpenCL_ C++ 2.2
kernel language committees.

This SYCL_ implementation is mainly based on C++1z (2017?) and OpenMP_
with execution on the CPU right now, but some parts of the non
single-source OpenCL_ interoperability layer are implemented and the
device compiler development is on-going for SPIR_ and SPIR-V_. Since in
SYCL_ there is a host fall-back, this CPU implementation can be
seen as an implementation of this fall-back too...

The parallel kernels can be executed in parallel on the CPU with OpenMP_ in
the first range dimension, if compiled with OpenMP_ support or on an
OpenCL_ device with the interoperability mode (which is not single source)

For legal reasons, the specification used for this open source project is
the published current provisional specification and not the last one
currently discussed in the Khronos_ OpenCL_ SYCL_ committee. If you are a
Khronos_ member, you can access to https://gitlab.khronos.org/sycl/triSYCL
where you might find more futuristic branches.

This is provided as is, without any warranty, with the same license as
LLVM_/Clang_.

Technical lead: Ronan at keryell point FR. Developments started first
at AMD_ and are now mainly funded by Xilinx_.


OpenCL SYCL
-----------

OpenCL_ SYCL_ is a single-source C++14/C++17-based DSEL_ (Domain Specific
Embedded Language) aimed at facilitating the programming of heterogeneous
accelerators by leveraging the OpenCL_ language and concepts.

Note that even if the concepts behind SYCL_ are inspired by OpenCL_
concepts, the SYCL_ programming model is a very general asynchronous
task graph model for heterogeneous computing with *no* relation with
OpenCL_ itself, except when using the OpenCL_ API interoperability
mode.

OpenCL_ SYCL_ is developed inside the Khronos_ OpenCL_ SYCL_ committee
and thus, for more information on SYCL_, look at
http://www.khronos.org/sycl

For the SYCL_ ecosystem, look at http://sycl.tech


Why you could use SYCL
~~~~~~~~~~~~~~~~~~~~~~

- Please see [about SYCL](doc/about-sycl.rst)


OpenCL triSYCL code documentation
---------------------------------

The documentation of the triSYCL_ implementation itself can be found in
http://xilinx.github.io/triSYCL/Doxygen/triSYCL/html and
http://xilinx.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf

Generating the Doxygen documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the top directory, run

.. code:: bash

  make

that will produce ``tmp/Doxygen/SYCL`` with the API documentation and
``tmp/Doxygen/triSYCL`` with the documented triSYCL_ implementation
source code.

To publish the documentation on GitHub:

.. code:: bash

  make publish

and finish as explained by the ``make`` output.


Possible futures
----------------

See `Possible futuress <doc/possible-futures.rst>`

News
----

- 2017/03/03: triSYCL can use CMake & ctest and works on Windows 10 with
  Visual Studio 2017. It works also with Ubuntu WSL on Windows. :-)
  `More info <doc/cmake.rst>`_

- 2017/01/12: Add test case using the Xilinx_ compiler for FPGA

- 2016/11/18: If you missed the free SYCL T-shirt on the Khronos booth
  during SC16_, you can always buy some on
  https://teespring.com/khronos-hpc (lady's sizes available, so no
  excuse! :-) )

- 2016/08/12: OpenCL_ kernels can be run with OpenCL kernel
  interoperability mode now.

- 2016/04/18: SYCL_ 2.2 provisional specification is out.

- This version implement SYCL_ 2.2 pipes and reservations plus the blocking
  pipe extension from Xilinx_.
