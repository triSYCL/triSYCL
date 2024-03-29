ACAP++ environment
++++++++++++++++++

..
  Not supported by GitHub :-(
  include:: doc/common-includes.rst

Some ACAP++ examples are found in `tests/acap </tests/acap>`_,
`tests/acap-buffer </tests/acap-buffer>`_, `tests/acap-lock
</tests/acap-lock>`_, `tests/acap-mem </tests/acap-mem>`_, `tests/air
</tests/air>`_.

The Doxygen-ized version of the API documentation can be found in the
precompiled
https://pages.gitenterprise.xilinx.com/rkeryell/acappp/Doxygen/acappp/html/group__acap.html
but you can also compile the latest version yourself as explained in
`doc/architecture.rst </doc/architecture.rst>`_


Installation
------------

Since it is based on modern C++, the best way to have everything
installed is to run Linux with the latest distribution, such as
Debian/unstable or Ubuntu 21.10, but it might work on older system too.

Such machines do exist at Xilinx, such as ``xsjsycl40.xilinx.com`` or
``xsjsycl41.xilinx.com`` we use to develop ACAP++ and you can ``ssh
-X`` into it to give a (graphical) try and skip directly to the next
section.

Some generic information about setting up such a machine or
environment can be found in
https://gitenterprise.xilinx.com/rkeryell/survival_kit

For example to install the latest Clang/LLVM, look at
http://apt.llvm.org

The most recent C++ compilers, the Boost library, GTKmm graphics C++
library are used.

To install them on latest Linux Debian/unstable (this should
work on latest Ubuntu too, just adapt the compiler versions):

.. code:: bash

  sudo apt-get install git make cmake clang-14 libomp-14-dev \
    llvm-14-dev g++-11 libboost-all-dev libgtkmm-3.0-dev libpocl-dev


Runtime environment
-------------------

Get ACAP++ itself with::

  git clone https://gitenterprise.xilinx.com/rkeryell/acappp.git


Some examples of ACAP++ uses a lot of stack storage and often the
default environment limits the amount to a small value such as 8 MiB,
which causes the program just to crash boldly when using more stack
allocation with a blunt message like::

  acap/wave_propagation
  Segmentation fault (core dumped)

To avoid this, increase the allowed stack size for example on a
``bash``-flavored shell::

  ulimit -s unlimited

or with a ``csh``-flavored shell::

  limit stacksize unlimited

There is also a bug in the graphics library preventing the graphics
environment to shutdown cleanly when closing the graphics window.
When it happens, just quit the main program, such as with ``^C``.


Compiling with Make
-------------------

To compile a given test program, go into the `tests </tests>`_
directory and execute::

  make acap/hello_world_uniform_lambda

and run the program with::

  acap/hello_world_uniform_lambda


Compiling with CMake
--------------------

Create a ``build`` directory somewhere and jump into it. From there,
run::

  CXX=clang++-14 cmake /where-the-acap++top-directory-is/ -DCMAKE_EXPORT_COMPILE_COMMANDS=1

Compile everything with::

  cmake --build tests/acap --parallel `nproc`

or just the ACAP main examples with::

  cmake --build tests/acap --parallel `nproc`

Then all you can run for example::

  tests/acap/acap_mandelbrot

Note that all executables have the test directory itself like
``acap_`` prepended to the name to avoid target name conflicts across
the directories for now.


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
