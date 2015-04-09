triSYCL
+++++++

.. section-numbering::

.. contents::


triSYCL is a humble implementation test-bed to experiment with the
provisional specification of the OpenCL_ SYCL_ `C++`_ layer and to give
feedback to the Khronos_ OpenCL_ SYCL_ committee.

For legal reasons, the specification used for this open source project is
the published current provisional specification and not the last one
currently discussed in the Khronos_ OpenCL_ SYCL_ committee. If you are a
Khronos_ member, you can ask for an access to
https://github.com/amd/triSYCL-private/tree/future in the
https://github.com/amd/triSYCL-private private repository to use a more
futuristic version.

This is just the start of a SYCL_ mock-up, only based on C++1z (2017?) and
OpenMP_ with execution on the CPU right now. So there is nothing related
to OpenCL_ yet. But since in SYCL_ there is a host fall-back, this can be
used as the start of this fall-back...

The parallel kernels can be executed in parallel on the CPU with OpenMP_ in
the first range dimension, if compiled with OpenMP support.

This is provided as is, without any warranty, with the same license as
LLVM_/Clang_.

Main contributor: Ronan Keryell at AMD_ point cOm


OpenCL SYCL
-----------

OpenCL_ SYCL_ is a single-source C++11-based DSEL_ (Domain Specific
Embedded Language) aimed at facilitating the programming of heterogeneous
accelerators by leveraging the OpenCL_ language and concepts.

OpenCL_ SYCL_ is developed inside the Khronos_ OpenCL_ SYCL_ committee and
thus, for more information on SYCL_, look at
http://www.khronos.org/opencl/sycl


Why you could use SYCL
~~~~~~~~~~~~~~~~~~~~~~

SYCL_ has a lot of interesting advantages compared to plain OpenCL_ or
other approaches:

- SYCL_ is an open standard from Khronos_ with a working committee (you can
  contribute!) and we can expect several implementations (commercial or
  open source) on many platforms soon, ranging from GPU, APU, FPGA... down
  to plain CPU;

- it offers a *single-source* `C++`_ programming model that allows taking
  advantage of the modern C++11 superpower, unifying both the host and
  accelerator sides. For example it is possible to write generic
  accelerated functions on the accelerators in a terse way by using
  (variadic) templates, meta-programming and lambda expressions;

- SYCL_ abstracts the concepts behind OpenCL_ and provides higher-level
  concepts such as ``command_group`` that allows the runtime to take
  advantage of a more task graph-oriented view of the computations. This
  allows lazy data transfers between accelerators and host or to use
  platform capabilities such as OpenCL 2 SVM or HSA_ for sharing data
  between host and accelerators;

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

- SYCL_ is high-level standard C++11 without any extension, that means
  that you can use your usual compiler and the host part can use some cool
  and common extensions such as OpenMP_, OpenHMPP_, OpenACC_,... or
  libraries such as MPI_ or PGAS Coarray++, be linked with other parts
  written in other languages (Fortran_...). Thus SYCL is already
  Exascale-ready!

- even if SYCL_ hides the OpenCL_ world by default, it inherits from all
  the OpenCL_ world:

  - same interoperability as the OpenCL_ underlying platform: OpenGL_,
    DirectX_...

  - access to all the underlying basic OpenCL_ objects behind the SYCL_
    abstraction for interoperability and hard-core optimization;

  - construction of SYCL_ objects from basic OpenCL_ objects to add some
    SYCL_ parts to an existing OpenCL_ application;

  - so it provides a continuum from higher-level programming `à la`
    `C++AMP`_ or OpenMP_ 4 down to low-level OpenCL_, according to the
    optimization needs, from using simple OpenCL intrinsics or vector
    operation from the ``cl::sycl`` namespace down to providing a real
    OpenCL kernel to be executed without requiring all the cumbersome
    usual OpenCL host API.

  This OpenCL seamless integration plus the gradual optimization features
  are perhaps the most compelling arguments for SYCL_ because it allows
  high-level programming simplicity without giving-up hard-core
  performance;

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

For introduction material on the interest of DSEL_ in this area, look for
example at these articles:

- `Domain-specific Languages and Code Synthesis Using Haskell
  <http://queue.acm.org/detail.cfm?id=2617811>`_, Andy Gill. May 6, 2014
  in ACM Queue and Communications of the ACM.

- `Design Exploration through Code-generating DSLs
  <http://queue.acm.org/detail.cfm?id=2626374>`_, Bo Joel Svensson, Mary
  Sheeran and Ryan Newton. May 15, 2014 in ACM Queue and Communications of
  the ACM.


Some presentations and publications related to SYCL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By reverse chronological order:

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


Some implementations
~~~~~~~~~~~~~~~~~~~~

Some other known implementations:

- CodePlay has an implementation based on OpenCL SPIR with Clang/LLVM
  http://codeplay.com/products


Related projects
~~~~~~~~~~~~~~~~

- Boost.Compute_

- VexCL_

- ViennaCL_

- Bolt_

- Thrust_

- `C++AMP`_

- Open Source implementation of C++AMP https://bitbucket.org/multicoreware/cppamp-driver-ng

- Intel Lab's iHRC https://github.com/IntelLabs/iHRC

- CUDA_

- C++ Parallelism TS https://github.com/cplusplus/parallelism-ts

- C++ Concurrency TS https://github.com/cplusplus/concurrency_ts


OpenCL triSYCL code documentation
---------------------------------

The documentation of the triSYCL implementation itself can be found in
http://amd.github.io/triSYCL/Doxygen/triSYCL/html and
http://amd.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf

An experimental description of the API generated from triSYCL through
Doxygen can be found in http://amd.github.io/triSYCL/Doxygen/SYCL/html and
http://amd.github.io/triSYCL/Doxygen/SYCL/SYCL-API-refman.pdf

But since the implementation has moved toward more meta-progamming usage,
this API documentation is no longer really descriptive of what is really
available, since it is mainly hidden by the meta-programming power. At
some point this API documentation will disappear.


Installation
------------

Only Clang_ 3.6+ or GCC_ 4.9+, `Boost.MultiArray`_ (which adds to `C++`_
the nice Fortran array semantics and syntax), Boost.Operators are needed.
If you use the debug mode or OpenMP, this works only with GCC 4.9 since
current Clang version does not support OpenMP yet.

To install them on latest Linux Debian/unstable (this should work on
latest Ubuntu too):

.. code:: bash

  sudo apt-get install clang-3.6 g++4.9 libboost-dev

There is nothing else to do for now to use the include files from
``include/CL`` when compiling a program. Just add a ``-I.../include/CL``
option when compiling.


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
``tmp/Doxygen/triSYCL`` with the documented ``triSYCL`` implementation
source code.

To publish the documentation on GitHub:

.. code:: bash

  make publish

and finish as explained by the ``make`` output.


Possible futures
----------------

Some ideas of future developments where *you* can contribute too: :-)

- finish implementation of basic classes without any OpenCL_ support;

- move to CMake for better portability;

- improve the test infrastructure;

- use the official OpenCL SYCL test suite to extend/debug/validate this
  implementation;

- add first OpenCL_ support with kernels provided only as strings, thus
  avoiding the need for a compiler. Could be based on other libraries such
  as Boost.Compute_, VexCL_ or Bolt_;

- make an accelerator version based on OpenMP_ 4 accelerator target,
  OpenHMPP_ or OpenACC_;

- make an accelerator version based on wrapper classes for the `C++AMP`_
  Open Source compiler
  https://bitbucket.org/multicoreware/cppamp-driver-ng

  Extend the current C++AMP OpenCL HSA or SPIR back-end runtime to expose
  OpenCL objects needed for the SYCL OpenCL interoperability. This is
  probably the simpler approach to have a running SYCL compiler working
  quickly.

  The main issue is that since C++AMP support is not yet integrated in the
  official trunk, it would take a long time to break things down and be
  reviewed by the Clang/LLVM community;

- extend runtime and Clang_/LLVM_ to generate OpenCL/SPIR_ from `C++`_
  single-source kernels, by using OpenMP outliner;

- alternatively develop a Clang/LLVM-based version, recycling the outliner
  which is already present for OpenMP support and modify it to generate
  SPIR. Then build a specific version of ``libiomp5`` to use the OpenCL
  C/C++ API to run the offloaded kernels.

  This approach may require more work than the C++AMP version but since it
  is based on the existing OpenMP infrastructure Intel spent a lot of time
  to upstream through the official code review process, at the end it
  would require quite less time for up-streaming, if this is the goal;

- add OpenCL_ 2.x support.


..
  Somme useful link definitions:

.. _AMD: http://www.amd/com

.. _Bolt: https://github.com/HSA-Libraries/Bolt

.. _Boost.Compute: http://kylelutz.github.io/compute/

.. _C++: http://www.open-std.org/jtc1/sc22/wg21/

.. _C++AMP: http://msdn.microsoft.com/en-us/library/hh265137.aspx

.. _Clang: http://clang.llvm.org/

.. _CUDA: https://developer.nvidia.com/cuda-zone

.. _DirectX: http://en.wikipedia.org/wiki/DirectX

.. _DSEL: http://en.wikipedia.org/wiki/Domain-specific_language

.. _Fortran: http://en.wikipedia.org/wiki/Fortran

.. _GCC: http://gcc.gnu.org/

.. _Boost.MultiArray: http://www.boost.org/doc/libs/1_55_0/libs/multi_array/doc/index.html

.. _HSA: http://www.hsafoundation.com/

.. _Khronos: https://www.khronos.org/

.. _LLVM: http://llvm.org/

.. _MPI: http://en.wikipedia.org/wiki/Message_Passing_Interface

.. _OpenACC: http://www.openacc-standard.org/

.. _OpenCL: http://www.khronos.org/opencl/

.. _OpenGL: https://www.khronos.org/opengl/

.. _OpenHMPP: http://en.wikipedia.org/wiki/OpenHMPP

.. _OpenMP: http://openmp.org/

.. _SPIR: http://www.khronos.org/spir

.. _SYCL: http://www.khronos.org/opencl/sycl/

.. _triSYCL: http://www.khronos.org/opencl/sycl/

.. _VexCL: http://ddemidov.github.io/vexcl/

.. _ViennaCL: http://viennacl.sourceforge.net


..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
