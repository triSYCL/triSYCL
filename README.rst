triSYCL
+++++++

This is a humble implementation test bed to experiment with the
provisional specification of the OpenCL SYCL C++ layer.

For more information on SYCL, look at http://www.khronos.org/opencl/sycl

This is just the start of a SYCL mock-up, only based on C++14 with
execution on the CPU. So there is nothing related to OpenCL yet. But since
in SYCL there is a host fall-back, this can be used as the start of this
fall-back...

The parallel kernels can be executed in parallel on the CPU with OpenMP in
the first range dimension, if compiled with OpenMP.

This is provided as is, without any warranty, with the same license as
LLVM/Clang.

Ronan Keryell at AMD point cOm


Installation
------------

Only Clang 3.5 or GCC 4.9 and Boost.MultiArray is needed.  If you use the
debug mode or OpenMP, this works only with GCC 4.9...

To install them on latest Linux Debian (this should work on latest Ubuntu
too):

.. code:: bash

  sudo apt-get install clang-3.5 g++4.9 libboost-dev


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

..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
