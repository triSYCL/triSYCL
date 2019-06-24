======================
 CMake infrastructure
======================

Features
========

Building
--------

The scripts intended to mimic the capabilities of the original GNU
Makefiles but enhance them with a little CMake awesome. The script
has the following options:

.. code:: CMake

    option(TRISYCL_OPENMP "triSYCL multi-threading with OpenMP" ON)
    option(TRISYCL_TBB "triSYCL multi-threading with TBB" OFF)
    option(TRISYCL_OPENCL "triSYCL OpenCL interoperability mode" OFF)
    option(TRISYCL_NO_ASYNC "triSYCL use synchronous kernel execution" OFF)
    option(TRISYCL_DEBUG "triSYCL use debug mode" OFF)
    option(TRISYCL_DEBUG_STRUCTORS "triSYCL trace of object lifetimes" OFF)
    option(TRISYCL_TRACE_KERNEL "triSYCL trace of kernel execution" OFF)
    option(TRISYCL_INCLUDE_DIR  "Use triSYCL include directory" OFF)


    set(CL_SYCL_LANGUAGE_VERSION 220 CACHE VERSION "Host language version to be used by triSYCL (default is: 220)")
    set(TRISYCL_CL_LANGUAGE_VERSION 220 CACHE VERSION "Device language version to be used by triSYCL (default is: 220) (not used yet)")

These variables can be set from the command-line at configuration
time. The default values I hope make sense for the vast majority of
development scenarios.

To compile the tests with OpenCL, use for example on Unix::

  mkdir build
  cd build
  cmake .. -DTRISYCL_OPENCL=ON
  # Use as many compilation threads as we have CPU cores
  # but it might be dangerous if there is not enough memory per core
  make -j`nproc`

Adding the option ``-DCMAKE_EXPORT_COMPILE_COMMANDS=1`` to the CMake
parameter is useful to generate the compilation database used by some
tools such as the Clang-based indexing and refactoring tools
(``run-clang-tidy``...).


Notes
`````

Enabling TBB (Intel Threading Building Blocks) will supersede OpenMP if both
options are enabled. Furthermore, when installed triSYCL will not specify any
particular backend. Thus if client applications want TBB to be enabled, then
they must specify ```-DTRISYCL_TBB``` and have TBB includes and linked libraries
properly set. A CMake module to find TBB can be found at
```cmake/FindTBB.cmake```, which is used when building the tests with TBB.


Testing
-------

The scripts also hook unit testing into the familiar CTest
framework. These scripts **do not rely on LIT** being present (one
less dependency), because none of the unit tests used any features
that are not present in CTest. CTest produces similarly sexy output as
LIT, can test exit codes and match stdout vs. a regex.

To run the tests once compiled, use for example::

  ctest


Warning-free
------------

As a good habit of mine, I raise the warning level on all compilers to
the highest value, and went ahead and disabled warnings specifically
that triggered in any of the tests. I would like to invite all devs to
take on their favorite compiler they use to develop/consume triSYCL
and get rid of all the warnings one by one. Some are fairly banal, but
some may be quite severe.

Notes
`````

The ``multiple_compilation_units`` tests fails on almost all compilers
with the output being scrambled even though the NO_BARRIER define is
set in code. What is the reason for this?

.. code:: CMake

  add_compile_options("/wd4459") # warning C4459: declaration of '<id>' hides global declaration
  add_compile_options("/wd4456") # warning C4456: declaration of '<id>' hides previous local declaration


For instance in the case of MSVC is either a two-phase name lookup
error or a potential source of a serious bug. These options can be
found in the top-level ``CMakeLists.txt`` file for all compilers.


Tested platforms
================

Windows 10 + VS 15 + Boost 1.63.0
---------------------------------

Importing the environment of a developer command prompt, (and having
the Ninja build system for eg. on the path), one can do something
like:

.. code:: PowerShell

  PS C:\Users\Matty\Build\triSYCL> Import-CmdEnvironment 'C:\Kellekek\Microsoft\Visual Studio\15RC\VC\Auxiliary\Build\vcvars64.bat'
  PS C:\Users\Matty\Build\triSYCL> cmake.exe -G"Ninja" -DBoost_COMPILER="-vc140" C:\Users\Matty\Source\Repos\triSYCL\

This creates Ninja makefiles that can be invoked as simply as:

.. code:: PowerShell

  PS C:\Users\Matty\Build\triSYCL> cmake --build .

which essentially invokes the underlying build systems 'all'
target. After build is complete, one can run tests simply by typing:

.. code:: PowerShell

  PS C:\Users\Matty\Build\triSYCL> ctest

which essentially invokes the underlying build systems 'test' target.


Notes
`````

Because the ``FindBoost.cmake`` scripts wrongly expected the toolset
of VS 15 to be v150 (instead of v141) one manually has to set the
toolset version by configuring using `-DBoost_COMPILER="-vc140"`. One
might ask: why 140 and not 141? Because even the coming Boost 1.64
does not compile with the new toolset, due to it having gone ahead and
riding the STL of deprecated STL functions such as |std::unary_function|_
which Boost does not handle yet.

..
  Some hack to have formatting + link
.. |std::unary_function| replace:: ``std::unary_function``
.. _std::unary_function: http://en.cppreference.com/w/cpp/utility/functional/unary_function


Ubuntu 16.04 (WSL) + GCC 6.2 + Boost 1.58.0
-------------------------------------------

Configure using:

.. code:: Bash

  mnagy@MATTY-Z50-75:~/build/triSYCL/gcc-6.2$ cmake -DCMAKE_C_COMPILER=gcc-6 -DCMAKE_CXX_COMPILER=g++-6 /mnt/c/Users/Matty/Source/Repos/triSYCL/

Building using:

.. code:: Bash

  mnagy@MATTY-Z50-75:~/build/triSYCL/gcc-6.2$ cmake --build . -- -j5

Testing:

.. code:: Bash

  mnagy@MATTY-Z50-75:~/build/triSYCL/gcc-6.2$ ctest


Ubuntu 16.04 (WSL) + Clang 4.0 + Boost 1.58.0
---------------------------------------------

Configure using:

.. code:: Bash

  mnagy@MATTY-Z50-75:~/build/triSYCL/clang-4.0$ cmake -DCMAKE_C_COMPILER=clang-4.0 -DCMAKE_CXX_COMPILER=clang++-4.0 -DTRISYCL_OPENMP=OFF /mnt/c/Users/Matty/Source/Repos/triSYCL/

Building using:

.. code:: Bash

  mnagy@MATTY-Z50-75:~/build/triSYCL/clang-4.0$ cmake --build . -- -j5

Testing:

.. code:: Bash

  mnagy@MATTY-Z50-75:~/build/triSYCL/clang-4.0$ ctest


Notes
`````

I could not get Clang actually work with OpenMP. It throws a runtime
(?!?!) exception for using unimplemented feature. Otherwise omitting
OpenMP results in some dead-locking tests.
