About SYCL
++++++++++

SYCL_ has a lot of interesting advantages compared to plain OpenCL_ or
other approaches:

- SYCL_ is an open standard from Khronos_ with a working committee
  (you can contribute!) and we can expect several implementations
  (commercial or open source) on many platforms soon, ranging from
  GPU, APU, FPGA, DSP... down to plain CPU;

- it offers a *single-source* `C++`_ programming model that allows
  taking advantage of the modern C++14/C++17 superpower, unifying both
  the host and accelerator sides. For example it is possible to write
  generic accelerated functions on the accelerators in a terse way by
  using (variadic) templates, meta-programming and generic variadic
  lambda expressions. This allows to build templated libraries such as
  Eigen_ or TensorFlow_ in a seamless way;

- SYCL_ abstracts and leverages the concepts behind OpenCL_ and provides
  higher-level concepts such as tasks (or command group in OpenCL_ SYCL_
  jargon) that allow the runtime to take advantage of a more task
  graph-oriented view of the computations. This allows lazy data
  transfers between accelerators and host or to use platform
  capabilities such as OpenCL_ 2 SVM or HSA_ for sharing data between
  host and accelerators;

- the entry cost of the technology is zero since, after all, an existing
  OpenCL_ or `C++`_ program is a valid SYCL_ program;

- the exit cost is low since it is *pure* `C++`_ *without any*
  extension or ``#pragma``, by opposition to `C++AMP`_ or OpenMP_ for
  example. Retargeting the SYCL_ classes and functions to use other
  frameworks such as OpenMP_ 4 or `C++AMP`_ is feasible without
  rewriting a new compiler for example;

- easier debugging

  - since all memory accesses to array parameters in kernels go through
    accessors, all the memory bound checks can be done in them if needed;

  - since there is a pure host mode, the kernel code can be run also on
    the host and debugged using the usual tools and use any system (such
    ``<cstdio>`` or ``<iostream>``...) or data libraries (for nice data
    visualization);

  - since the kernel code is `C++`_ code even when run on an accelerator,
    instrumenting the code by using special array classes or overloading
    some operators allows deep intrusive debugging or code analysis
    without changing the algorithmic parts of the code;

- SYCL_ is high-level standard modern `C++`_ without any extension, that means
  that you can use your usual compiler and the host part can use at
  the *same time* some cool and common extensions such as OpenMP_,
  OpenHMPP_, OpenACC_,... or libraries such as MPI_ or PGAS Coarray++,
  be linked with other parts written in other languages
  (Fortran_...). Thus SYCL_ is already Exascale-ready!

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
    optimization needs, from using simple OpenCL_ intrinsics or vector
    operation from the ``cl::sycl`` namespace down to providing a real
    OpenCL_ kernel to be executed without requiring all the verbose
    usual OpenCL_ host API.

  This OpenCL_ seamless integration plus the gradual optimization features
  are perhaps the most compelling arguments for SYCL_ because it allows
  high-level programming simplicity without giving-up hard-core
  performance when needed;

- since the SYCL_ task graph execution model is asynchronous, this can be
  used by side effect to overcome some underlying OpenCL_ implementation
  limitations. For example, some OpenCL_ stacks may have only in-order
  execution queues or even synchronous (blocking) ND-range enqueue, or
  some weird constrained mapping between OpenCL_ programmer level queue(s)
  and the hardware queues.

  In this case, a SYCL_ implementation can deal with this, relying for
  example on multiple host CPU threads, multiple thread-local-storage
  (TLS) queues, its own scheduler, etc. atop the limited OpenCL_ stack to
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

- `SYCL C++ and OpenCL interoperability experimentation with triSYCL
  <https://github.com/keryell/ronan/raw/gh-pages/publications/conf/2017/IWOCL-DHCPP/triSYCL/2017-05-16-IWOCL-DHPCC-triSYCL.pdf>`_.
  Anastasios Doumoulakis, Ronan Keryell & Kenneth
  O'Brien. `Distributed & Heterogeneous Programming for C/C++ workshop
  (DHPCC++ 2017)
  <http://sycl.tech/distributed-heterogeneous-programming-in-c-cpp-dhpccpp17.html>`_,
  collocated with `IWOCL'17
  <http://www.iwocl.org/iwocl-2017/conference-program/>`_, Toronto,
  Canada. May 16, 2017.

- `Post-modern C++17 abstractions for heterogeneous computing with
  Khronos OpenCL SYCL
  <https://github.com/keryell/ronan/raw/gh-pages/Talks/2017/2017-04-10-Dublin_C++_meetup_SYCL/2017-04-10-Dublin_C++_meetup_SYCL-expose.pdf>`_.
  Ronan Keryell. `Dublin C++ User Group Meetup
  <https://www.meetup.com/cppdug/events/238673768/>`_,
  Dublin, Ireland. April 10, 2017.

- `Post-modern C++17 abstractions for heterogeneous computing with
  Khronos OpenCL SYCL
  <https://github.com/keryell/ronan/raw/gh-pages/Talks/2017/2017-01-19-Paris_C++_meetup_SYCL/2017-01-19-Paris_C++_meetup_SYCL-expose.pdf>`_.
  Ronan Keryell. `Paris C++ User Group Meetup
  <https://www.meetup.com/User-Group-Cpp-Francophone/events/236788136/>`_,
  Paris, France. January 19, 2017.

- `Khronos Group SYCL standard --- triSYCL Open Source Implementation
  <https://github.com/keryell/ronan/raw/gh-pages/Talks/2016/2016-11-14--16-Khronos_SC16_Booth_SYCL/2016-11-14--16-Khronos_SC16_Booth_SYCL-expose.pdf>`_,
  Ronan Keryell (Xilinx & Khronos OpenCL SYCL Working Group Member).
  November, 2016, Presentation at `SuperComputing 2016
  <http://sc16.supercomputing.org>`_, Salt Lake City, USA.

- `P0367R0: Accessors — wrapper classes to qualify accesses
  <http://ronan.keryell.fr/Talks/2016/2016-06-25-C%2B%2B_accessors_Oulu/2016-06-25-C%2B%2B_accessors_Oulu-expose.pdf>`_,
  Ronan Keryell (Xilinx) & Joël Falcou (NumScale). November, 2016,
  Presentation at ISO C++ committee, Issaquah, WA, USA.

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

- `C++`_ ISO/IEC JTC1/SC22/WG21 WG21 committee_

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

