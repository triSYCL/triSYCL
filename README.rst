triSYCL
+++++++

This is a humble implementation test bed to experiment with the
provisional specification of the OpenCL_ SYCL_ `C++`_ layer and to give
feedback to the Khronos_ OpenCL_ SYCL_ committee.

This is just the start of a SYCL_ mock-up, only based on C++14 and OpenMP_
with execution on the CPU right now. So there is nothing related to
OpenCL_ yet. But since in SYCL_ there is a host fall-back, this can be used
as the start of this fall-back...

The parallel kernels can be executed in parallel on the CPU with OpenMP_ in
the first range dimension, if compiled with OpenMP support.

This is provided as is, without any warranty, with the same license as
LLVM_/Clang_.

Main contributor: Ronan Keryell at AMD_ point cOm


OpenCL triSYCL API
---------------

An experimental description of the API generated from triSYCL through
Doxygen can be found in http://amd.github.io/triSYCL/Doxygen/SYCL/html and
http://amd.github.io/triSYCL/Doxygen/SYCL/SYCL-API-refman.pdf

The documentation of the triSYCL implementation itself can be found in
http://amd.github.io/triSYCL/Doxygen/triSYCL/html and
http://amd.github.io/triSYCL/Doxygen/triSYCL/triSYCL-implementation-refman.pdf


OpenCL SYCL
-----------

OpenCL_ SYCL_ is a C++11-based DSEL_ (Domain Specific Embedded Language)
aimed at facilitating the programming of heterogeneous accelerators by
leveraging the OpenCL_ language and concepts.

For more information on SYCL_, look at http://www.khronos.org/opencl/sycl

For introduction material on the interest of DSEL_ in this area, look for
example at these articles:

- *Domain-specific Languages and Code Synthesis Using Haskell*, Andy
  Gill. May 6, 2014 in ACM Queue and Communications of the ACM
  http://queue.acm.org/detail.cfm?id=2617811

- *Design Exploration through Code-generating DSLs*, Bo Joel Svensson,
  Mary Sheeran and Ryan Newton. May 15, 2014 in ACM Queue and
  Communications of the ACM http://queue.acm.org/detail.cfm?id=2626374


Some advantages of SYCL
~~~~~~~~~~~~~~~~~~~~~~~


SYCL_ has a lot of interesting advantages compared to plain OpenCL_ or
other approaches:

- SYCL_ is an open standard from Khronos_ with a working committee (you can
  contribute!) and we can expect several implementations (commercial or
  open source) on many platforms soon;

- it offers a single-source `C++`_ programming model that allows taking
  advantage of the modern C++11 superpower unifying both the host and
  accelerator sides. For example it is possible to write generic
  accelerated functions on the accelerators in a terse way by using
  (variadic) templates and lambda expressions;

- SYCL_ abstracts the concepts behind OpenCL_ and provides higher-level
  concepts such as ``command_group`` that allows the runtime to take
  advantage of a more graph-oriented view of the computations. This allows
  lazy data transfers between accelerators and host or to use platform
  capabilities such as HSA_ for sharing data between host and
  accelerators;

- the entry cost of the technology is zero since, after all, an existing
  OpenCL_ or `C++`_ program is a valid SYCL_ program;

- the exit cost is low since it is pure `C++`_ without extension or
  ``#pragma``. Retargeting the SYCL_ classes and functions to use other
  frameworks such as OpenMP_ 4 or `C++AMP`_ is feasible without rewriting a new
  compiler for example;

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

- SYCL_ is high-level standard C++11 without any extension, that means that
  you can use your usual compiler and the host part can use extensions
  such as OpenMP_, OpenHMPP_, OpenACC_,... or libraries such as MPI_, be
  linked with other parts written in other languages (Fortran_...);

- SYCL_ inherit from all the OpenCL_ world:

  - same interoperability as the OpenCL_ underlying platform: OpenGL_,
    DirectX_...

  - access to all the underlying basic OpenCL_ objects behind the SYCL_
    abstraction for interoperability and hard-core optimization;

  - construction of SYCL_ objects from basic OpenCL_ objects to add some
    SYCL_ parts to an existing OpenCL_ application;

  - so it provides a continuum from higher-level programming `à la` `C++AMP`_
    or OpenMP_ 4 down to low-level OpenCL_, according to the optimization
    needs, which seems the most compelling argument for SYCL_.


Some presentations on SYCL
~~~~~~~~~~~~~~~~~~~~~~~~~~

By reverse chronological order:

- *OpenCL 2.0, OpenCL SYCL & OpenMP 4, open standards for heterogeneous
  parallel programming*, Ronan Keryell (AMD & Khronos OpenCL Working Group
  Member). 3 July, 2014, High Performance Computing & Supercomputing Group
  of Paris.
  http://www.meetup.com/HPC-GPU-Supercomputing-Group-of-Paris-Meetup/events/185216422/

- *OpenCL 2.0, OpenCL SYCL & OpenMP 4, open standards for heterogeneous
  parallel programming*, Ronan Keryell (AMD & Khronos OpenCL Working Group
  Member). 2 July, 2014, Forum Ter@tec: *Calcul scientifique & Open Source
  : pratiques industrielles des logiciels libres*.
  http://www.teratec.eu/forum/atelier_3.html

- *SYCL : Abstraction Layer for Leveraging C++ and OpenCL*, Maria Rovatsou
  (Codeplay & Khronos OpenCL Working Group Member). 12-13 May, 2014,
  IWOCL 2014.
  http://iwocl.org/iwocl-2014/abstracts/sycl-abstraction-layer-for-leveraging-c-and-opencl/

- *Building the OpenCL ecosystem - SYCL for OpenCL*, Lee Howes (Senior
  Staff Engineer at Qualcomm & Khronos OpenCL Working Group Member). 21
  April, 2014, HPC & GPU Supercomputing Group of Silicon
  Valley. http://www.meetup.com/HPC-GPU-Supercomputing-Group-of-Silicon-Valley/events/151429932/

- *SYCL 1.2: Unofficial High-Level Overview*, AJ Guillon (Khronos OpenCL
  Working Group Member). 19
  March, 2014. https://www.youtube.com/watch?v=-mEQhf8MeUI

- *SYCL for OpenCL*, Andrew Richards (CEO Codeplay & Chair SYCL Working
  group). 19 March, 2014, GDC 2014.
  http://www.khronos.org/assets/uploads/developers/library/2014-gdc/SYCL-for-OpenCL-GDC-Mar14.pdf


Installation
------------

Only Clang_ 3.5 or GCC_ 4.9 and `Boost.MultiArray`_ (which adds to `C++`_ the
nice Fortran array semantics and syntax) is needed.  If you use the debug
mode or OpenMP, this works only with GCC 4.9 since current Clang version
does not support OpenMP yet.

To install them on latest Linux Debian/unstable (this should work on
latest Ubuntu too):

.. code:: bash

  sudo apt-get install clang-3.5 g++4.9 libboost-dev


Doxygen documentation
~~~~~~~~~~~~~~~~~~~~~

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


Examples
--------

There are trivial examples in the ``tests`` directory.

To compile them:

.. code:: bash

  make

To run them:

.. code:: bash

  make run


To clean up:

.. code:: bash

  make clean

You can build the binary with different compilers with

.. code:: bash

  make CXX=g++-4.9
  make CXX=clang++


Future
------

Some ideas of future developments where *you* can contribute:

- split the declaration from the implementation so that the documentation
  of the SYCL API itself can be generated by Doxygen or some other tools
  from this project;

- finish implementation of basic classes without any OpenCL_ support;

- add a test infrastructure;

- make an accelerator version based on wrapper classes for the `C++AMP`_
  Open Source compiler
  https://bitbucket.org/multicoreware/cppamp-driver-ng/wiki/Home

- make an accelerator version based on OpenMP_ 4, OpenHMPP_ or OpenACC_;

- add OpenCL_ support with kernels only provided as strings;

- extend runtime and Clang_/LLVM_ to generate OpenCL/SPIR_ from `C++`_
  single-source kernels;

- add OpenCL_ 2.x support.


..
  Somme useful link definitions:

.. _AMD: http://www.amd/com

.. _C++: http://www.open-std.org/jtc1/sc22/wg21/

.. _C++AMP: http://msdn.microsoft.com/en-us/library/hh265137.aspx

.. _Clang: http://clang.llvm.org/

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


..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
