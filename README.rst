triSYCL
+++++++

..
  Not supported by GitHub :-(
  include:: doc/common-includes.rst

.. section-numbering::

.. highlight:: C++

..
  Add a badge with the build status of the CI
  Cf https://docs.github.com/en/actions/managing-workflow-runs/adding-a-workflow-status-badge
.. image:: https://github.com/triSYCL/triSYCL/actions/workflows/cmake.yml/badge.svg
    :target: https://github.com/triSYCL/triSYCL/actions

Introduction
------------

triSYCL_ is a research project to experiment with the specification of
the SYCL_ standard and to give feedback to the Khronos_ Group
SYCL_committee and also to the ISO `C++`_ committee.

More recently, this project has been used to experiment with high-level C++
programming for AMD_ AIE CGRA inspired by some SYCL concepts, as explained in
`ACAP++ & AIE++: C++ extensions for AMD Versal AIE CGRA architecture`_.

Because of lack of resources **this SYCL implementation is very
incomplete and should not be used by a normal end-user.** Fortunately
there are now many other implementations of SYCL_ available, including
some strong implementations like ComputeCpp_, `DPC++`_ or hipSYCL_
that can be used on various targets.

This implementation is mainly based on C++23 features backed with
OpenMP_ or TBB_ for parallel execution on the CPU, with
`Boost.Compute`_ for the non single-source OpenCL_ interoperability
layer and with an experimental LLVM_/Clang_ version for the device
compiler (from 2017-2018 which is now obsolete) providing full
single-source SYCL_ experience, typically targeting a SPIR_
device. Since in SYCL_ there is a host fall-back, this CPU
implementation can be seen as an implementation of this fall-back too.

Since around 2018 Intel has put a lot of effort in their own oneAPI
`DPC++`_ SYCL_ project to up-stream SYCL_ into LLVM_/Clang_, there is
another project about merging the oneAPI `DPC++`_ SYCL_ implementation
with triSYCL_ at https://github.com/triSYCL/sycl to give a greater
user experience for Xilinx_ FPGA instead of using our obsolete
experimental clunky device compiler. But this is still very
experimental because the Xilinx_ tool-chain is based on old
incompatible versions of LLVM_/Clang_ and nothing of these is
supported by the Xilinx_ product teams.

triSYCL_ has been used to experiment and provide feedback for SYCL_
1.2, 1.2.1, 2.2, 2020 and even the OpenCL_ C++ 1.0 kernel language
from OpenCL_ 2.2.

This is provided as is, without any warranty, with the same license as
LLVM_/Clang_.

Technical lead: Ronan at keryell point FR. Developments started first
at AMD_, then was mainly funded by Xilinx_ and now again by AMD_ since
Xilinx_ has been bought by AMD_ in 2022.

It is possible to have a paid internship around triSYCL, if you have
some skills related to this project. Contact the technical lead about
this. AMD_ is also hiring in this area... :-)


SYCL
----

SYCL_ is a single-source modern C++-based DSEL_ (Domain Specific
Embedded Language) and open standard from Khronos_ aimed at
facilitating the programming of heterogeneous accelerators by
leveraging existing concepts inspired by OpenCL_, CUDA_, `C++AMP`_, OpenMP_...

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

SYCL_ is developed inside the Khronos_ SYCL_ committee and thus, for
more information on SYCL_, look at https://www.khronos.org/sycl

Note that even if the concepts behind SYCL_ are inspired by OpenCL_
concepts, the SYCL_ programming model is a very general asynchronous
task graph model for heterogeneous computing targeting various
frameworks and API and has *no* relation with OpenCL_ itself, except
when using the OpenCL_ API interoperability mode, like any other
target.

For the SYCL_ ecosystem, look at https://sycl.tech


ACAP++ & AIE++: C++ extensions for AMD Versal AIE CGRA architecture
-------------------------------------------------------------------

Most of our current efforts are focused on extensions, such as targeting AMD_
FPGA and Versal ACAP AIE CGRA, providing a way to program CPU, GPU, FPGA and
CGRA at the same time in a single-source C++ program.

This project is a work-in-progress and currently we target partially only the
first generation of devices, AIE/AIE1, while current models of AMD_ RyzenAI such
as the Ryzen 9 7940HS has an AIE-ML/AIE2 as the XDNA/NPU/IPU.

- The first generation programming model, `ACAP++` was based on C++17/C++20
  constructs.

  See
  https://github.com/triSYCL/sycl/blob/sycl/unified/master/sycl/test/acap/test_aie_mandelbrot.cpp
  and around, `<tests/acap>`_ and other directories starting with `acap` for
  some code samples running in pure C++ library CPU emulation with this project.

  Look at `<doc/acap.rst>`_ to know more about how to install/use the ACAP++
  environment.

  The runtime for CPU emulation and AIE device is found in
  `<include/triSYCL/vendor/Xilinx>`_ which requires also a special compiler
  provided by https://github.com/triSYCL/sycl to run on VCK190 boards.

- The second generation programming model, `AIE++` is based on C++23/C++26
  constructs, allowing an even terser syntax.

  See around
  `<https://github.com/triSYCL/sycl/blob/sycl/unified/master/sycl/test/aie/mandelbrot.cpp>`_
  for some examples.

  The runtime for CPU emulation and AIE device support is found in
  `<include/aie>`_ and the compiler for device support is
  https://github.com/triSYCL/sycl

Documentation
-------------

Some reasons to use SYCL
~~~~~~~~~~~~~~~~~~~~~~~~

Please see `about SYCL <doc/about-sycl.rst>`_ to have some context, a
list of presentations, some related projects.


Installation & testing
~~~~~~~~~~~~~~~~~~~~~~

SYCL_ is a template library, so no real installation is required.

There are some examples you can build however.

See `Testing <doc/testing.rst>`_.


Architecture of triSYCL runtime and compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`Architecture of triSYCL runtime and compiler <doc/architecture.rst>`_
describes the code base with some high-level diagrams but also how it
was possible to compile and use the obsolete device compiler on some Xilinx_
FPGA for example. Now look at https://github.com/triSYCL/sycl instead.


CMake infrastructure
~~~~~~~~~~~~~~~~~~~~

Some details about CMake configuration and organization can be found
in `CMake <doc/cmake.rst>`_.


Pre-processor macros used in triSYCL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Yes, there are some macros used in triSYCL! Look at `Pre-processor
macros used in triSYCL <doc/macros.rst>`_ to discover some of them.


Environment variables used in triSYCL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

See `Environment variables with triSYCL <doc/environment.rst>`_.


Possible futures
~~~~~~~~~~~~~~~~

See `Possible futures <doc/possible-futures.rst>`_.


triSYCL code documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~

The documentation of the triSYCL_ implementation itself can be found
in https://trisycl.github.io/triSYCL/Doxygen/triSYCL/html and
https://trisycl.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf

There are also some internal documentation at
https://pages.gitenterprise.xilinx.com/rkeryell/acappp/Doxygen/acappp/html


News
----

- 2023/06/09: merge the 5-year old branch experimenting with ACAP++
  SYCL CPU model extensions for AMD_ Versal ACAP AIE1 CGRA like the
  XCVC1902 used in VCK190 or VCK5000 boards.

- 2018/03/12: the long-going device compiler branch has been merged in
  to provide experimental support for SPIR-df friendly devices, such
  as PoCL_ or Xilinx_ FPGA. This is only for the brave for now.

- 2018/02/01: there is now some documentation about the architecture of
  triSYCL on GPU and accelerators with its device compiler based on
  Clang_/LLVM_ in `<doc/architecture.rst>`_. While this is wildly
  experimental, there is a growing interest around it and it is
  always useful to get started as a contributor.

- 2018/01/05: there are some internship openings at Xilinx_ to work on
  triSYCL for FPGA
  https://xilinx.referrals.selectminds.com/jobs/compiler-engineer-intern-on-sycl-for-fpga-4685
  and more generally Xilinx_ is hiring in compilation, runtime, C++,
  SYCL_, OpenCL_, machine-learning...

- 2017/12/06: the brand-new SYCL 1.2.1 specification is out and
  triSYCL starts moving to it

- 2017/11/17: the presentations and videos from `SC17
  <https://sc17.supercomputing.org>`_ on SYCL and triSYCL are now
  online https://www.khronos.org/news/events/supercomputing-2017

- 2017/09/19: there is a prototype of device compiler based on
  Clang_/LLVM_ generating SPIR 2.0 "de facto" (SPIR-df) and working at least
  with PoCL_ and Xilinx_ SDx `xocc` for FPGA.

- 2017/03/03: triSYCL can use CMake & ``ctest`` and works on Windows 10 with
  Visual Studio 2017. It works also with Ubuntu WSL on Windows. :-)
  `More info <doc/cmake.rst>`_

- 2017/01/12: Add test case using the Xilinx_ compiler for FPGA

- 2016/11/18: If you missed the free SYCL_ T-shirt on the Khronos booth
  during SC16_, you can always buy some on
  https://teespring.com/khronos-hpc (lady's sizes available, so no
  excuse! :-) )

- 2016/08/12: OpenCL_ kernels can be run with OpenCL_ kernel
  interoperability mode now.

- 2016/04/18: SYCL_ 2.2 provisional specification is out. This version
  implement SYCL_ 2.2 pipes and reservations plus the blocking pipe
  extension from Xilinx_.


..
  Actually include:: doc/common-includes.rst does not work in GitHub
  :-( https://github.com/github/markup/issues/172

  So manual inline of the following everywhere... :-(

.. Some useful link definitions:

.. _AMD: https://www.amd.com

.. _Bolt: https://github.com/HSA-Libraries/Bolt

.. _Boost.Compute: https://github.com/boostorg/compute

.. _C++: https://www.open-std.org/jtc1/sc22/wg21/

.. _committee: https://isocpp.org/std/the-committee

.. _C++AMP: https://msdn.microsoft.com/en-us/library/hh265137.aspx

.. _Clang: https://clang.llvm.org/

.. _CLHPP: https://github.com/KhronosGroup/OpenCL-CLHPP

.. _Codeplay: https://www.codeplay.com

.. _ComputeCpp: https://www.codeplay.com/products/computesuite/computecpp

.. _CUDA: https://developer.nvidia.com/cuda-zone

.. _DirectX: https://en.wikipedia.org/wiki/DirectX

.. _DPC++: https://github.com/intel/llvm/tree/sycl

.. _DSEL: https://en.wikipedia.org/wiki/Domain-specific_language

.. _Eigen: https://eigen.tuxfamily.org

.. _Fortran: https://en.wikipedia.org/wiki/Fortran

.. _GCC: https://gcc.gnu.org/

.. _GOOPAX: https://www.goopax.com/

.. _HCC: https://github.com/RadeonOpenCompute/hcc

.. _HIP: https://github.com/ROCm-Developer-Tools/HIP

.. _hipSYCL: https://github.com/illuhad/hipSYCL

.. _HSA: https://www.hsafoundation.com/

.. _Khronos: https://www.khronos.org/

.. _LLVM: https://llvm.org/

.. _Metal: https://developer.apple.com/library/ios/documentation/Metal/Reference/MetalShadingLanguageGuide

.. _MPI: https://en.wikipedia.org/wiki/Message_Passing_Interface

.. _OpenACC: https://www.openacc-standard.org/

.. _OpenAMP: https://www.multicore-association.org/workgroup/oamp.php

.. _OpenCL: https://www.khronos.org/opencl/

.. _OpenGL: https://www.khronos.org/opengl/

.. _OpenHMPP: https://en.wikipedia.org/wiki/OpenHMPP

.. _OpenMP: https://openmp.org/

.. _PACXX: https://pacxx.github.io/page/

.. _PoCL: https://portablecl.org/

.. _SYCL Parallel STL: https://github.com/KhronosGroup/SyclParallelSTL

.. _RenderScript: https://en.wikipedia.org/wiki/Renderscript

.. _SC16: https://sc16.supercomputing.org

.. _SG14: https://groups.google.com/a/isocpp.org/forum/?fromgroups=#!forum/sg14

.. _SPIR: https://www.khronos.org/spir

.. _SPIR-V: https://www.khronos.org/spir

.. _SYCL: https://www.khronos.org/sycl

.. _TensorFlow: https://www.tensorflow.org

.. _TBB: https://www.threadingbuildingblocks.org/

.. _Thrust: https://thrust.github.io/

.. _triSYCL: https://github.com/triSYCL/triSYCL

.. _VexCL: https://ddemidov.github.io/vexcl/

.. _ViennaCL: https://viennacl.sourceforge.net/

.. _Vulkan: https://www.khronos.org/vulkan/

.. _Xilinx: https://www.xilinx.com

..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
