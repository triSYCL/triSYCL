Possible Futures
++++++++++++++++

.. include:: common-includes.rst

There are already some ideas on GitHub issues,
https://github.com/triSYCL/triSYCL/issues
and internally to Xilinx
https://gitenterprise.xilinx.com/rkeryell/triSYCL/issues

Some ideas of future developments where *you* can contribute too: :-)

- go on using the official OpenCL_ SYCL_ conformance test suite (CTS)
  which is now open-source https://github.com/KhronosGroup/SYCL-CTS to
  extend/debug/validate this implementation;

- finish implementation of basic classes without any OpenCL_ support
  to have SYCL 1.2.1 passing the CTS on CPU;

- go on extending the device compiler, based on Clang_/LLVM_ and some
  specific runtime, to generate OpenCL_/SPIR_ from `C++`_
  single-source kernels;

- move to CMake even for the device compiler for better portability
  the tests;

- improve the test infrastructure (for example move to something more
  standard with Boost.Test. Status: started) instead of having some
  tests using LIT to test some textual output;

- update the dataflow SYCL infrastructure from plain C++ ``std::thread``
  and ``std::condition_variable`` to some more efficient library, such
  as TBB_ or `Boost.Fiber <https://github.com/boostorg/fiber>`_;

- implement the dataflow SYCL infrastructure directly on top of OpenCL
  event-framework instead of layering the CPU dataflow dependency
  graph on top of it;

- add vector swizzle support by following ideas from
  https://github.com/gwiazdorrr/CxxSwizzle http://glm.g-truc.net
  http://jojendersie.de/performance-optimal-vector-swizzling-in-c
  http://www.reedbeta.com/blog/2013/12/28/on-vector-math-libraries ;

- add OpenCL_ 2.x support;

- since SYCL_ is a pretty general programming model for heterogeneous
  computing, if the OpenCL_ compatibility layer is not required, some other
  back-ends could be written besides the current OpenMP_ one: CUDA_,
  RenderScript_, OpenAMP_, etc. Actually this is the base for hipSYCL_

- make an alternative accelerator version based on OpenMP_ 5
  accelerator target, OpenHMPP_ or OpenACC_;

- make an alternative accelerator version based on wrapper classes for
  the `C++AMP`_ HCC_ Open Source compiler;

- SYCL_ concepts (well, classes) can also be ported to some other
  languages to provide heterogeneous support: SYJSCL, SYCamlCL,
  SYJavaCL... It is not clear yet if SYFortranCL is possible with
  Fortran 2008 or 2015+;

- some of the proposed extensions above are actually on-going is some
  other projects or in the fusion of Intel SYCL and triSYCL
  https://github.com/triSYCL/sycl


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

.. _HCC: https://github.com/RadeonOpenCompute/hcc

.. _HIP: https://github.com/ROCm-Developer-Tools/HIP

.. _hipSYCL: https://github.com/illuhad/hipSYCL

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

.. _TBB: https://www.threadingbuildingblocks.org/

.. _Thrust: http://thrust.github.io/

.. _triSYCL: https://github.com/triSYCL/triSYCL

.. _VexCL: http://ddemidov.github.io/vexcl/

.. _ViennaCL: http://viennacl.sourceforge.net/

.. _Vulkan: https://www.khronos.org/vulkan/

.. _Xilinx: http://www.xilinx.com
