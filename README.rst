triSYCL
+++++++

..
  Not supported by GitHub :-(
  include:: doc/common-includes.rst

.. highlight:: C++

.. image:: https://travis-ci.org/triSYCL/triSYCL.svg?branch=master
    :target: https://travis-ci.org/triSYCL/triSYCL

Introduction
------------

triSYCL_ is an open source implementation to experiment with
the specification of the OpenCL_ SYCL_ 1.2.1 and 2.2 `C++`_ layer and
to give feedback to the Khronos_ OpenCL_ SYCL_ and OpenCL_ C++ 2.2
kernel language committees and also ISO `C++`_ committee.

This SYCL_ implementation is mainly based on C++17 and OpenMP_ for
execution on the CPU, with `Boost.Compute`_ for the non single-source
OpenCL_ interoperability layer and with LLVM_/Clang_ for the device
compiler providing single-source experience on SPIR_ device. Since in
SYCL_ there is a host fall-back, this CPU implementation can be seen
as an implementation of this fall-back too...

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

It is possible to have a paid internship around triSYCL, if you have
some skills related to this project. Contact the technical lead about
this. Xilinx in also hiring in this area... :-)


OpenCL SYCL
-----------

OpenCL_ SYCL_ is a single-source C++14/C++17-based DSEL_ (Domain Specific
Embedded Language) aimed at facilitating the programming of heterogeneous
accelerators by leveraging the OpenCL_ language and concepts.

A typical kernel with its launch looks like this pure modern C++ code::

  queue {}.submit([&](handler &h) {
      auto accA = bufA.get_access<access::mode::read>(h);
      auto accB = bufB.get_access<access::mode::write>(h);
      h.parallel_for<class myKernel>(myRange, [=](item i) {
          accA[i] = accB[i] + 1;
      });
  });

Look for example at
https://github.com/triSYCL/triSYCL/blob/master/tests/examples/demo_parallel_matrix_add.cpp
for a complete example.

Note that even if the concepts behind SYCL_ are inspired by OpenCL_
concepts, the SYCL_ programming model is a very general asynchronous
task graph model for heterogeneous computing with *no* relation with
OpenCL_ itself, except when using the OpenCL_ API interoperability
mode.

OpenCL_ SYCL_ is developed inside the Khronos_ OpenCL_ SYCL_ committee
and thus, for more information on SYCL_, look at
http://www.khronos.org/sycl

For the SYCL_ ecosystem, look at http://sycl.tech


Documentation

Some reasons to use SYCL
~~~~~~~~~~~~~~~~~~~~~~~~

- Please see `about SYCL <doc/about-sycl.rst>`_


Installation & testing
----------------------

SYCL is a template library, so no real installation is required.

There are some examples you can build however.

See `Testing <doc/testing.rst>`_


OpenCL triSYCL code documentation
---------------------------------

The documentation of the triSYCL_ implementation itself can be found in
http://xilinx.github.io/triSYCL/Doxygen/triSYCL/html and
http://xilinx.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


Possible futures
----------------

See `Possible futures <doc/possible-futures.rst>`_


News
----

- 2018/03/12: the long-going device compiler branch has been merged in
  to provide experimental support for SPIR-df friendly devices, such
  as PoCL or Xilinx FPGA. This is only for the brave for now.

- 2018/02/01: there is now some documentation about the architecture of
  triSYCL on GPU and accelerators with its device compiler based on
  Clang/LLVM in `<doc/architecture.rst>`_. While this is wildly
  experimental, there is a growing interested around it and it is
  always useful to get started as a contributor.

- 2018/01/05: there are some internship openings at Xilinx to work on
  triSYCL for FPGA
  https://xilinx.referrals.selectminds.com/jobs/compiler-engineer-intern-on-sycl-for-fpga-4685
  and more generally Xilinx is hiring in compilation, runtime, C++,
  SYCL, OpenCL, machine-learning...

- 2017/12/06: the brand-new SYCL 1.2.1 specification is out and
  triSYCL starts moving to it

- 2017/11/17: the presentations and videos from `SC17
  <http://sc17.supercomputing.org>`_ on SYCL and triSYCL are now
  online https://www.khronos.org/news/events/supercomputing-2017

- 2017/09/19: there is a prototype of device compiler based on
  Clang_/LLVM_ generating SPIR 2.0 "de facto" and working at least
  with PoCL and Xilinx SDx `xocc` for FPGA. Look at the `device`
  branch in https://github.com/triSYCL/triSYCL/tree/device

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

- 2016/04/18: SYCL_ 2.2 provisional specification is out. This version
  implement SYCL_ 2.2 pipes and reservations plus the blocking pipe
  extension from Xilinx_.


..
  Actually include:: doc/common-includes.rst does not work in GitHub
  :-( https://github.com/github/markup/issues/172

  So manual inline of the following everywhere... :-(

.. Some useful link definitions:

.. _AMD: http://www.amd.com

.. _Bolt: https://github.com/HSA-Libraries/Bolt

.. _Boost.Compute: https://github.com/boostorg/compute

.. _Boost.MultiArray: http://www.boost.org/doc/libs/1_55_0/libs/multi_array/doc/index.html

.. _C++: http://www.open-std.org/jtc1/sc22/wg21/

.. _committee: https://isocpp.org/std/the-committee

.. _C++AMP: http://msdn.microsoft.com/en-us/library/hh265137.aspx

.. _Clang: http://clang.llvm.org/

.. _CLHPP: https://github.com/KhronosGroup/OpenCL-CLHPP

.. _Codeplay: http://www.codeplay.com

.. _ComputeCpp: https://www.codeplay.com/products/computesuite/computecpp

.. _CUDA: https://developer.nvidia.com/cuda-zone

.. _DirectX: http://en.wikipedia.org/wiki/DirectX

.. _DSEL: http://en.wikipedia.org/wiki/Domain-specific_language

.. _Eigen: http://eigen.tuxfamily.org

.. _Fortran: http://en.wikipedia.org/wiki/Fortran

.. _GCC: http://gcc.gnu.org/

.. _GOOPAX: http://www.goopax.com/

.. _HSA: http://www.hsafoundation.com/

.. _Khronos: https://www.khronos.org/

.. _LLVM: http://llvm.org/

.. _Metal: https://developer.apple.com/library/ios/documentation/Metal/Reference/MetalShadingLanguageGuide

.. _MPI: http://en.wikipedia.org/wiki/Message_Passing_Interface

.. _OpenACC: http://www.openacc-standard.org/

.. _OpenAMP: https://www.multicore-association.org/workgroup/oamp.php

.. _OpenCL: http://www.khronos.org/opencl/

.. _OpenGL: https://www.khronos.org/opengl/

.. _OpenHMPP: http://en.wikipedia.org/wiki/OpenHMPP

.. _OpenMP: http://openmp.org/

.. _PACXX: http://pacxx.github.io/page/

.. _SYCL Parallel STL: https://github.com/KhronosGroup/SyclParallelSTL

.. _RenderScript: http://en.wikipedia.org/wiki/Renderscript

.. _SC16: http://sc16.supercomputing.org

.. _SG14: https://groups.google.com/a/isocpp.org/forum/?fromgroups=#!forum/sg14

.. _SPIR: http://www.khronos.org/spir

.. _SPIR-V: http://www.khronos.org/spir

.. _SYCL: https://www.khronos.org/sycl

.. _TensorFlow: https://www.tensorflow.org

.. _Thrust: http://thrust.github.io/

.. _triSYCL: https://github.com/triSYCL/triSYCL

.. _VexCL: http://ddemidov.github.io/vexcl/

.. _ViennaCL: http://viennacl.sourceforge.net/

.. _Vulkan: https://www.khronos.org/vulkan/

.. _Xilinx: http://www.xilinx.com

..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
