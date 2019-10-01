triSYCL testing
+++++++++++++++

.. section-numbering::

.. contents::

Here are some simple examples and tests using triSYCL.

Everything is under control of either

- a native GNU `Makefile`

- or a `CMake`-based configuration.


Environment variables
=====================

OpenCL tests use Boost.Compute and thus you can use to select the
OpenCL device/environment/... you want to use by setting some of the
following environment variables:

- ``BOOST_COMPUTE_DEFAULT_DEVICE``

- ``BOOST_COMPUTE_DEFAULT_DEVICE_TYPE``

- ``BOOST_COMPUTE_DEFAULT_PLATFORM``

- ``BOOST_COMPUTE_DEFAULT_VENDOR``

- ``BOOST_COMPUTE_DEFAULT_ENFORCE``

For example to use PoCL http://portablecl.org

.. code:: bash

  export BOOST_COMPUTE_DEFAULT_PLATFORM="Portable Computing Language"

To specify where the OpenCL include files and libraries are found
outside of the standard directories, you can use the following
variables:

- ``OpenCL_INCPATH`` specifies where ``CL/cl.h`` can be found, for example;

- ``OpenCL_LIBPATH`` specifies where to find the OpenCL library

The Boost.Compute package has a specific importance for this project
and sometime it is useful to test for a more modern version. This can
be searched at a specific place through the following environment
variable:

- ``BOOST_COMPUTE_INCPATH``


The tests based on the Makefile are using LLVM_ LIT_, not the CMake
ones, using ``ctest``. So, when using the Makefile tests, you need to
specify where is LIT_. For example with:

.. code:: bash

  export TRISYCL_LIT=/usr/lib/llvm-8/build/utils/lit/lit.py

Installing the Xilinx SDx software providing OpenCL support for Xilinx
FPGA sets normally the ``XILINX_SDX`` environment variable. This is
used by the Makefile to run the Xilinx tests.


Test-specific Makefile variables
================================

``TARGETS`` allows to specify the test targets to work on, instead of
deriving them from all the ``.cpp`` files found.

You can set the flags given to Xilinx OpenCL ``xocc`` compiler with
the ``XOCCFLAGS`` variable.

When using LIT_ tests from the Makefile, you can use

- ``CHECKING_DIR`` to specify which sub-directory to run the tests
  from, instead of all the tests found in the current ``tests``
  directory;

- ``LITFLAGS`` to pass some options to LIT_.


Compiling and execution
=======================

Using ``CMake``
---------------

See `../doc/cmake.rst <../doc/cmake.rst>`_


Using ``make`` directly
-----------------------

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

  make CXX=g++-9
  make CXX=clang++-9

To compile with some debug options, look around some setting of the
``CXXFLAGS`` variable. You can configure triSYCL through various
macros described in `../doc/macros.rst <../doc/macros.rst>`_

To restrict the ``Makefile`` effect on some specific test cases, override
the ``TARGETS`` variable with space-separated list of the binaries you
want to run, such as:

.. code:: bash

  make TARGETS=examples/demo_parallel_matrices_add run


Testing
=======

There are actually 2 ways to run the unit tests, either using a
Makefile/LIT path or a CMake file, because some experimental tests can
only run with ``make`` for now (device compiler & FPGA...).

So to have an idea about which tests are run with which path, try for
example::

  # Display the list of tests using LIT:
  make check-list
  # Display the list of tests using CMake & ctest:
  make ctest-list


Using ``CMake`` ``ctest``
-------------------------

See `../doc/cmake.rst <../doc/cmake.rst>`_

Basically, just use ``ctest``.


Using ``make`` and ``LIT``
--------------------------

The tests are based on the LIT_ tool from the LLVM_ infrastructure.

To install it on Debian or Ubuntu, use typically:

.. code:: bash

  sudo apt-get install llvm-8-tools

You should direct the following variable to where is LIT_ located on your
machine, for example:

.. code:: bash

  export TRISYCL_LIT=/usr/lib/llvm-8/build/utils/lit/lit.py

To run with the checker, try:

.. code:: bash

  make check

Normally you should clean up before with a:

.. code:: bash

  make clean

to be sure everything is recompiled from the current source. It is not
made by default because when developing, you may keep some already
compiled version with some specific flags, for example.

You can set the ``LITFLAGS`` variable to run LIT_ with some options,
such as ``-v``. The ``CHECKING_DIR`` variable restrict the test on the
given directory. For example:

.. code:: bash

  make LITFLAGS=--verbose CHECKING_DIR=pipe clean check

To run the tests on several default compilers (G++, Clang...) and cleaning
in the between, try:

.. code:: bash

  make check-compilers

To check into a new work-tree to be sure you did not forget to commit some
files, use the ``clone-T`` targets, such as:

.. code:: bash

  make clone-check


..
  Somme useful link definitions:

.. _LIT: http://llvm.org/docs/CommandGuide/lit.html
.. _LLVM: http://llvm.org/


..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
