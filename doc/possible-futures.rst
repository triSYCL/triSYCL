Possible Futures
++++++++++++++++

Some ideas of future developments where *you* can contribute too: :-)

- finish implementation of basic classes without any OpenCL_ support;

- move to CMake for better portability (status: Lee Howes has made it on 1 of
  his private branches. To be merged...);

- improve the test infrastructure (for example move to something more
  standard with Boost.Test. Status: started);

- use the official OpenCL_ SYCL_ test suite to extend/debug/validate this
  implementation;

- add vector swizzle support by following ideas from
  https://github.com/gwiazdorrr/CxxSwizzle http://glm.g-truc.net
  http://jojendersie.de/performance-optimal-vector-swizzling-in-c
  http://www.reedbeta.com/blog/2013/12/28/on-vector-math-libraries ;

- add first OpenCL_ support with kernels provided only as strings,
  thus avoiding the need for a compiler. Could be based on other
  libraries such as Boost.Compute_, VexCL_, ViennaCL_... (status:
  started with Boost.Compute_);

- make an accelerator version based on OpenMP_ 4 accelerator target,
  OpenHMPP_ or OpenACC_;

- make an accelerator version based on wrapper classes for the `C++AMP`_
  Open Source compiler.

  Extend the current `C++AMP`_ OpenCL_ HSA_ or SPIR_ back-end runtime
  to expose OpenCL_ objects needed for the SYCL_ OpenCL_
  interoperability. This is probably the simpler approach to have a
  running SYCL_ compiler working quickly.

  The main issue is that since `C++AMP`_ support is not yet integrated
  in the official trunk, it would take a long time to break things
  down and be reviewed by the Clang/LLVM_ community. Actually, since
  Microsoft is no longer pushing this project and there are some
  design issues in the language requiring a lot of change to the C++
  parser, it will probably never be up-streamed to Clang/LLVM;

- extend runtime and Clang_/LLVM_ to generate OpenCL_/SPIR_ from `C++`_
  single-source kernels, by using OpenMP_ outliner. Starting from an open
  source OpenCL_ C/C++ compiler sounds great;

- alternatively develop a Clang_/LLVM_-based version, recycling the
  outliner which is already present for OpenMP_ support and modify it
  to generate SPIR_. Then build a specific version of ``libiomp5`` to
  use the OpenCL_ C/C++ API to run the offloaded kernels. See
  https://drive.google.com/file/d/0B-jX56_FbGKRM21sYlNYVnB4eFk/view
  and the projects https://github.com/clang-omp/libomptarget for
  https://github.com/clang-omp/llvm_trunk and
  https://github.com/clang-omp/clang_trunk

  This approach may require more work than the `C++AMP`_ version but
  since it is based on the existing OpenMP_ infrastructure Intel spent
  a lot of time to upstream through the official code review process,
  at the end it would require quite less time for up-streaming, if
  this is the goal.

  OpenMP4 in Clang_/LLVM_ is getting momentum and making lot of
  progress backed by Intel, IBM, AMD... so it sounds like a way to
  go;

- recycle the GCC https://gcc.gnu.org/wiki/Offloading OpenMP_/OpenACC_
  library infrastructure to construct an OpenCL_ interoperability API and
  adapt the triSYCL_ classes to leverage OpenMP_/OpenACC_;

- add OpenCL_ 2.x support with SYCL_ 2.x;

- since SYCL_ is a pretty general programming model for heterogeneous
  computing, if the OpenCL_ compatibility layer is not required, some other
  back-ends could be written besides the current OpenMP_ one: CUDA_,
  RenderScript_, etc.

- SYCL_ concepts (well, classes) can also be ported to some other
  languages to provide heterogeneous support: SYJSCL, SYCamlCL,
  SYJavaCL... It is not clear yet if SYFortranCL is possible with
  Fortran 2008 or 2015+.

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

