triSYCL
+++++++

News
----

- 2016/11/18: If you missed the free SYCL T-shirt on the Khronos booth
  during SC16_, you can always buy some on
  https://teespring.com/khronos-hpc (lady's sizes available!)

- 2016/08/12: OpenCL kernels can be run with OpenCL kernel
  interoperability mode now.

- 2016/04/18: SYCL 2.2 provisional specification is out.

- This version implement SYCL 2.2 pipes and reservations plus the blocking
  pipe extension from Xilinx_.

.. section-numbering::


Table of content
----------------

.. contents::


Introduction
------------

triSYCL_ is an implementation test-bed to experiment with the
specification of the OpenCL_ SYCL_ 2.2 `C++`_ layer and to give feedback
to the Khronos_ OpenCL_ SYCL_ and OpenCL C++ kernel language
committees.

This SYCL_ implementation is only based on C++1z
(2017?) and OpenMP_ with execution on the CPU right now. So there is
almost nothing related to OpenCL_ or SPIR_ yet. But since in SYCL_
there is a host fall-back, this can be seen as an implementation of
this fall-back too...

The parallel kernels can be executed in parallel on the CPU with OpenMP_ in
the first range dimension, if compiled with OpenMP_ support.

For legal reasons, the specification used for this open source project is
the published current provisional specification and not the last one
currently discussed in the Khronos_ OpenCL_ SYCL_ committee. If you are a
Khronos_ member, you can access to https://gitlab.khronos.org/sycl/triSYCL
where you can find more futuristic branches.

This is provided as is, without any warranty, with the same license as
LLVM_/Clang_.

Technical lead: Ronan at keryell point FR. Developments started first
at AMD_ and are now mainly funded by Xilinx_.


OpenCL SYCL
-----------

OpenCL_ SYCL_ is a single-source C++14-based DSEL_ (Domain Specific
Embedded Language) aimed at facilitating the programming of heterogeneous
accelerators by leveraging the OpenCL_ language and concepts.

OpenCL_ SYCL_ is developed inside the Khronos_ OpenCL_ SYCL_ committee and
thus, for more information on SYCL_, look at
http://www.khronos.org/sycl


Why you could use SYCL
~~~~~~~~~~~~~~~~~~~~~~

SYCL_ has a lot of interesting advantages compared to plain OpenCL_ or
other approaches:

- SYCL_ is an open standard from Khronos_ with a working committee
  (you can contribute!) and we can expect several implementations
  (commercial or open source) on many platforms soon, ranging from
  GPU, APU, FPGA, DSP... down to plain CPU;

- it offers a *single-source* `C++`_ programming model that allows taking
  advantage of the modern C++14 superpower, unifying both the host and
  accelerator sides. For example it is possible to write generic
  accelerated functions on the accelerators in a terse way by using
  (variadic) templates, meta-programming and lambda expressions. This
  allows to build templated libraries such as Eigen_ or TensorFlow_ in
  a seamless way;

- SYCL_ abstracts the concepts behind OpenCL_ and provides
  higher-level concepts such as tasks (or command group in OpenCL SYCL
  jargon) that allow the runtime to take advantage of a more task
  graph-oriented view of the computations. This allows lazy data
  transfers between accelerators and host or to use platform
  capabilities such as OpenCL 2 SVM or HSA_ for sharing data between
  host and accelerators;

- the entry cost of the technology is zero since, after all, an existing
  OpenCL_ or `C++`_ program is a valid SYCL_ program;

- the exit cost is low since it is *pure* `C++`_ *without any* extension
  or ``#pragma``. Retargeting the SYCL_ classes and functions to use other
  frameworks such as OpenMP_ 4 or `C++AMP`_ is feasible without rewriting
  a new compiler for example;

- easier debugging

  - since all memory accesses to array parameters in kernels go through
    accessors, all the memory bound checks can be done in them if needed;

  - since there is a pure host mode, the kernel code can be run also on
    the host and debugged using the usual tools and use any system (such
    ``stdio`` or ``iostream``...) or data libraries (for nice data
    visualization);

  - since the kernel code is `C++`_ code even when run on an accelerator,
    instrumenting the code by using special array classes or overloading
    some operators allows deep intrusive debugging or code analysis
    without changing the algorithmic parts of the code;

- SYCL_ is high-level standard C++14 without any extension, that means
  that you can use your usual compiler and the host part can use at
  the *same time* some cool and common extensions such as OpenMP_,
  OpenHMPP_, OpenACC_,... or libraries such as MPI_ or PGAS Coarray++,
  be linked with other parts written in other languages
  (Fortran_...). Thus SYCL is already Exascale-ready!

- even if SYCL_ hides the OpenCL_ world by default, it inherits from all
  the OpenCL_ world:

  - same interoperability as the OpenCL_ underlying platform: Vulkan_,
    OpenGL_, DirectX_...

  - access to all the underlying basic OpenCL_ objects behind the SYCL_
    abstraction for interoperability and hard-core optimization;

  - construction of SYCL_ objects from basic OpenCL_ objects to add some
    SYCL_ parts to an existing OpenCL_ application;

  - so it provides a continuum from higher-level programming `à la`
    `C++AMP`_ or OpenMP_ 4 down to low-level OpenCL_, according to the
    optimization needs, from using simple OpenCL intrinsics or vector
    operation from the ``cl::sycl`` namespace down to providing a real
    OpenCL kernel to be executed without requiring all the verbose
    usual OpenCL host API.

  This OpenCL seamless integration plus the gradual optimization features
  are perhaps the most compelling arguments for SYCL_ because it allows
  high-level programming simplicity without giving-up hard-core
  performance when needed;

- since the SYCL task graph execution model is asynchronous, this can be
  used by side effect to overcome some underlying OpenCL implementation
  limitations. For example, some OpenCL stacks may have only in-order
  execution queues or even synchronous (blocking) ND-range enqueue, or
  some weird constrained mapping between OpenCL programmer level queue(s)
  and the hardware queues.

  In this case, a SYCL implementation can deal with this, relying for
  example on multiple host CPU threads, multiple thread-local-storage
  (TLS) queues, its own scheduler, etc. atop the limited OpenCL stack to
  provide computation and communication overlap in a natural pain-free
  fashion. This relieves the programmer to reorganize her application to
  work around these limitation, which can be quite a cumbersome work.

for introduction material on the interest of DSEL_ in this area, look for
example at these articles:

- `Domain-specific Languages and Code Synthesis Using Haskell
  <http://queue.acm.org/detail.cfm?id=2617811>`_, Andy Gill. May 6, 2014
  in ACM Queue and Communications of the ACM.

- `Design Exploration through Code-generating DSLs
  <http://queue.acm.org/detail.cfm?id=2626374>`_, Bo Joel Svensson, Mary
  Sheeran and Ryan Newton. May 15, 2014 in ACM Queue and Communications of
  the ACM.


Some other implementations
~~~~~~~~~~~~~~~~~~~~~~~~~~

Some other known implementations:

- Codeplay_ has a ComputeCpp_ product implementing SYCL_ based on
  OpenCL_ SPIR_ with Clang_/LLVM_
  https://www.codeplay.com/products/computesuite/computecpp

- SYCL-GTX https://github.com/ProGTX/sycl-gtx


Some presentations and publications related to SYCL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By reverse chronological order:

- `Experiments with triSYCL: poor (wo)man shared virtual memory
  <https://github.com/keryell/ronan/raw/gh-pages/Talks/2016/2016-03-13-PPoPP-SYCL-triSYCL/2016-03-13-PPoPP-SYCL-triSYCL-expose.pdf>`_.
  Ronan Keryell. `SYCL 2016 - 1st SYCL Programming Workshop
  <http://conf.researchr.org/track/SYCL-2016/SYCL-2016-papers>`_,
  collocated with `PPoPP'16 <http://ppopp16.sigplan.org/>`_,
  Barcelona, Spain. March 13, 2016.

- `Khronos's OpenCL SYCL to support Heterogeneous Devices for C++
  <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0236r0.pdf>`_.
  Proposal for the C++ committee SG14 in Jacksonville, Florida, USA
  February 12, 2016.

- `SYCL presentation at SG14 C++ committee teleconference
  <https://groups.google.com/a/isocpp.org/group/sg14/attach/10071129554d08/SYCL%20to%20SG14%20Presentation.pdf?part=0.1>`_,
  Andrew Richards (CEO Codeplay & Chair SYCL Working group). February
  3, 2016.

- `Post-modern C++ abstractions for FPGA & heterogeneous computing
  with OpenCL SYCL & SPIR-V
  <https://github.com/keryell/ronan/raw/gh-pages/Talks/2016/2016-01-21--22-ANL-REFORM/2016-01-22-ANL-REFORM-Xilinx_SYCL_SPIR-V-expose.pdf>`_,
  Ronan Keryell. ANL REFORM 2016: Workshop on FPGAs for scientific
  simulation and data analytics, Argone National Labs. January
  22, 2016.

- `From modern FPGA to high-level post-modern C++ abstractions for
  heterogeneous computing with OpenCL SYCL & SPIR-V
  <https://github.com/keryell/ronan/raw/gh-pages/Talks/2016/HiPEAC-WRC-2016/2016-01-19-HiPEAC-WRC-Xilinx_FPGA_SYCL_keynote-expose.pdf>`_,
  Ronan Keryell. HiPEAC WRC 2016: Workshop on Reconfigurable
  Computing. Prague, January 19, 2016.

- `HiPEAC 2016 tutorial on SYCL: Khronos SYCL for OpenCL
  <https://www.hipeac.net/events/activities/7328/sycl/#fndtn-program>`_.
  HiPEAC 2016, Prague, January 18, 2016.

- `A Tutorial on Khronos SYCL for OpenCL at IWOCL 2015
  <http://codeplaysoftware.github.io/iwocl2015>`_. Stanford, May 12, 2015.

- `Modern C++, OpenCL SYCL & OpenCL CL2.hpp
  <http://ronan.keryell.fr/Talks/2014/2014-11-18-SC14-OpenCL_BoF_SYCL/2014-11-18-OpenCL_BoF_SYCL-expose.pdf>`_,
  Ronan Keryell (AMD & Khronos OpenCL SYCL Working Group Member).
  November 18, 2014, Presentation at `SuperComputing 2014, OpenCL BoF
  <http://sc14.supercomputing.org/schedule/event_detail?evid=bof131>`_,
  New Orleans, USA.

- `Implementing the OpenCL SYCL Shared Source C++ Programming Model using
  Clang/LLVM
  <http://www.codeplay.com/public/uploaded/publications/SC2014_LLVM_HPC.pdf>`_,
  Gordon Brown. November 17, 2014, `Workshop on the LLVM Compiler
  Infrastructure in HPC, SuperComputing 2014
  <http://llvm-hpc-workshop.github.io>`_

- `SYCL Specification --- SYCL integrates OpenCL devices with modern C++
  <https://www.khronos.org/registry/sycl/specs/sycl-1.2.pdf>`_, Khronos
  OpenCL Working Group — SYCL subgroup. Editors: Lee Howes and Maria
  Rovatsou. Version 1.2, Revision 2014-09-16.

- `OpenCL 2.0, OpenCL SYCL & OpenMP 4, open standards for heterogeneous
  parallel programming
  <http://ronan.keryell.fr/Talks/2014/2014-07-03-Paris_HPC_GPU_meetup/2014-07-03-Paris-HPC-GPU-Meetup-RK-expose.pdf>`_,
  Ronan Keryell (AMD & Khronos OpenCL Working Group Member). July 3, 2014,
  Presentation at the Meetup of the `High Performance Computing &
  Supercomputing Group of Paris
  <http://www.meetup.com/HPC-GPU-Supercomputing-Group-of-Paris-Meetup/events/185216422>`_.

- `OpenCL 2.0, OpenCL SYCL & OpenMP 4, open standards for heterogeneous
  parallel programming
  <http://ronan.keryell.fr/Talks/2014/2014-07-02-Ter@Tec/Ter@tec-HC-RK-expose.pdf>`_,
  Ronan Keryell (AMD & Khronos OpenCL Working Group Member). July 2, 2014,
  Presentation at Forum Ter\@tec: `Calcul scientifique & Open Source :
  pratiques industrielles des logiciels libres
  <http://www.teratec.eu/forum/atelier_3.html>`_.

- `The Future of Accelerator Programming in C++
  <https://github.com/boostcon/cppnow_presentations_2014/blob/master/files/CppNow2014_Future_of_Accelerator_Programming.pdf?raw=true>`_,
  Sebastian Schaetz, May 18, 2014. Presentation at `C++Now 2014
  <http://cppnow.org/schedule-2014>`_.

- *SYCL : Abstraction Layer for Leveraging C++ and OpenCL*, Maria Rovatsou
  (Codeplay & Khronos OpenCL Working Group Member). May 12-13, 2014,
  `IWOCL 2014
  <http://iwocl.org/iwocl-2014/abstracts/sycl-abstraction-layer-for-leveraging-c-and-opencl>`_.

- *Building the OpenCL ecosystem - SYCL for OpenCL*, Lee Howes (Senior
  Staff Engineer at Qualcomm & Khronos OpenCL Working Group Member).
  April 21, 2014, `HPC & GPU Supercomputing Group of Silicon Valley
  <http://www.meetup.com/HPC-GPU-Supercomputing-Group-of-Silicon-Valley/events/151429932>`_.

- `SYCL 1.2: Unofficial High-Level Overview
  <https://www.youtube.com/watch?v=-mEQhf8MeUI>`_, AJ Guillon (Khronos
  OpenCL Working Group Member). March 19, 2014. Video.

- `SYCL for OpenCL
  <http://www.khronos.org/assets/uploads/developers/library/2014-gdc/SYCL-for-OpenCL-GDC-Mar14.pdf>`_,
  Andrew Richards (CEO Codeplay & Chair SYCL Working group). March 19,
  2014, GDC 2014.

- `Fusing GPU kernels within a novel single-source C++ API
  <http://lpgpu.org/wp/wp-content/uploads/2014/02/PEGPUM_2014_codeplay.pdf>`_,
  Ralph Potter, Paul Keir, Jan Lucas, Maurico Alvarez-Mesa, Ben Juurlink
  and Andrew Richards. January 20, 2014, LPGPU Workshop on Power-Efficient
  GPU and Many-core Computing (PEGPUM 2014).

- `Fusing GPU kernels within a novel single-source C++ API
  <https://software.intel.com/sites/default/files/managed/c5/45/Confpkeir_haifa_compilers_architectures_tools.pdf>`_,
  Ralph Potter, Paul Keir, Jan Lucas, Mauricio Alvarez-Mesa, Ben Juurlink,
  Andrew Richards. November 18, 2013, Intel Compiler, Architecture and
  Tools Conference.

There are also many interesting articles in the `publication list from
Codeplay <http://www.codeplay.com/company/publications.html>`_.


Related projects
~~~~~~~~~~~~~~~~

- CLHPP_: The OpenCL C++ wrapper from Khronos around host API

- Boost.Compute_

- VexCL_

- ViennaCL_

- `C++`_ WG21 committee_

  - the SG14_ subgroup on low Latency, real time requirements,
    performance, efficiency, heterogeneous computing, where SYCL is
    one of the candidates;

  - C++ Parallelism TS https://github.com/cplusplus/parallelism-ts

    `SYCL Parallel STL`_ is an implementation of the Parallel STL of
    C++17 based on SYCL;

  - C++ Concurrency TS https://github.com/cplusplus/concurrency_ts

- OpenMP_ is a ``#pragma``-based standard to express different kind of
  parallelism with accelerators supported since version 4.0;

- OpenACC_ is a ``#pragma``-based extension targetting accelerators;

- Bolt_

- Thrust_

- `C++AMP`_

- HCC https://bitbucket.org/multicoreware/hcc/wiki/Home

- GOOPAX_ is a product providing a C++11 framework for single-source
  OpenCL;

- PACXX_ is a higher-level C++ compiler and framework for accelerators;

- Intel SPMD Program Compiler https://ispc.github.io/

- Intel Lab's iHRC https://github.com/IntelLabs/iHRC

- CUDA_

- Metal_


OpenCL triSYCL code documentation
---------------------------------

The documentation of the triSYCL_ implementation itself can be found in
http://xilinx.github.io/triSYCL/Doxygen/triSYCL/html and
http://xilinx.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


Installation
------------

Only Clang_ 3.9+ or GCC_ 5.2+, `Boost.MultiArray`_ (which adds to `C++`_
the nice Fortran array semantics and syntax), Boost.Operators and a few
other Boost libraries are needed.

To install them on latest Linux Debian/unstable (this should work on
latest Ubuntu too, just adapt the compiler versions):

.. code:: bash

  sudo apt-get install clang-3.9 g++-6 libboost-dev

There is nothing else to do for now to use the include files from triSYCL_
``include`` directory when compiling a program. Just add a
``-I.../include`` option and ``-std=c++1y`` when compiling.

Also use ``-fopenmp`` if you want to use multicore parallelism on the CPU.


Examples and tests
------------------

There are simple examples and tests in the ``tests`` directory.  Look at
`tests/README.rst` description.


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

Some ideas of future developments where *you* can contribute too: :-)

- finish implementation of basic classes without any OpenCL_ support;

- move to CMake for better portability (status: Lee Howes has made it on 1 of
  his private branches. To be merged...);

- improve the test infrastructure (for example move to something more
  standard with Boost.Test. Status: started);

- use the official OpenCL SYCL test suite to extend/debug/validate this
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

  Extend the current C++AMP OpenCL HSA or SPIR back-end runtime to expose
  OpenCL objects needed for the SYCL OpenCL interoperability. This is
  probably the simpler approach to have a running SYCL compiler working
  quickly.

  The main issue is that since C++AMP support is not yet integrated in the
  official trunk, it would take a long time to break things down and be
  reviewed by the Clang/LLVM community;

- extend runtime and Clang_/LLVM_ to generate OpenCL/SPIR_ from `C++`_
  single-source kernels, by using OpenMP outliner. Starting from an open
  source OpenCL C/C++ compiler sounds great;

- alternatively develop a Clang/LLVM-based version, recycling the outliner
  which is already present for OpenMP support and modify it to generate
  SPIR. Then build a specific version of ``libiomp5`` to use the OpenCL
  C/C++ API to run the offloaded kernels. See
  https://drive.google.com/file/d/0B-jX56_FbGKRM21sYlNYVnB4eFk/view and
  the projects https://github.com/clang-omp/libomptarget for
  https://github.com/clang-omp/llvm_trunk and
  https://github.com/clang-omp/clang_trunk

  This approach may require more work than the C++AMP version but since it
  is based on the existing OpenMP infrastructure Intel spent a lot of time
  to upstream through the official code review process, at the end it
  would require quite less time for up-streaming, if this is the goal.

  OpenMP4 in Clang/LLVM is getting momentum and making lot of progress
  backed by Intel, IBM, AMD... so it sounds like the way to go;

- recycle the GCC https://gcc.gnu.org/wiki/Offloading OpenMP/OpenACC
  library infrastructure to construct an OpenCL interoperability API and
  adapt the triSYCL classes to leverage OpenMP/OpenACC;

- add OpenCL_ 2.x support with SYCL 2.x;

- since SYCL is a pretty general programming model for heterogeneous
  computing, if the OpenCL compatibility layer is not required, some other
  back-ends could be written besides the current OpenMP_ one: CUDA_,
  RenderScript_, etc.

- SYCL concepts (well, classes) can also be ported to some other
  languages to provide heterogeneous support: SYJSCL, SYCamlCL,
  SYJavaCL... It is not clear yet if SYFortranCL is possible with
  Fortran 2008 or 2015+.

..
  Somme useful link definitions:

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

.. _SYCL: https://www.khronos.org/sycl

.. _TensorFlow: https://www.tensorflow.org

.. _Thrust: http://thrust.github.io/

.. _triSYCL: https://github.com/Xilinx/triSYCL

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
