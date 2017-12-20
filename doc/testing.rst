Testing
+++++++

.. include:: common-includes.rst

Installation
------------

Only Clang_ 3.9+ or GCC_ 5.4+, `Boost.MultiArray`_ (which adds to `C++`_
the nice Fortran array semantics and syntax), Boost.Operators and a few
other Boost libraries are needed.

To install them on latest Linux Debian/unstable (this should work on
latest Ubuntu too, just adapt the compiler versions):

.. code:: bash

  sudo apt-get install clang-3.9 g++-6 libboost-dev

There is nothing else to do for now to use the include files from triSYCL_
``include`` directory when compiling a program. Just add a
``-I.../include`` option and ``-std=c++1y`` when compiling.

triSYCL_ is configurable through preprocessor macros described in
`macros <macros.rst>`_.

Also use ``-fopenmp`` if you want to use multicore parallelism on the CPU.

The ``CMake`` support is described in `cmake.rst <cmake.rst>`_.


Examples and tests
------------------

There are simple examples and tests in the `tests </tests>`_ directory.
Look at `tests/README.rst </tests/README.rst>`_ description.
