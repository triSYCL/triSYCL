triSYCL testing
+++++++++++++++

.. section-numbering::

.. contents::

Here are some simple examples and tests using triSYCL.

Everything is under control of the GNU `Makefile`.


Compiling and execution
=======================


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

To compile with some debug options, look around some setting of the
``CXXFLAGS`` variable.

To restrict the ``Makefile`` effect on some specific test cases, override
the ``TARGETS`` variable with space-separated list of the binaries you
want to run, such as:

.. code:: bash

  make TARGETS=examples/demo_parallel_matrices_add run


Testing
=======

The tests are based on the LIT_ tool from the LLVM_ infrastructure.

To install it on Debian or Ubuntu, use typically:

.. code:: bash

  sudo apt-get install llvm-3.5-tools

You should direct the following variable to where is LIT located on your
machine, for example:

.. code:: bash

  export TRISYCL_LIT=/usr/lib/llvm-3.5/build/utils/lit/lit.py

To run with the checker, try:

.. code:: bash

  make check

Normally you should clean up before with a:

.. code:: bash

  make clean

to be sure everything is recompiled from the current source. It is not
made by default because when developing, you may keep some already
compiled version with some specific flags, for example.

You can set the ``LITFLAGS`` variable to run LIT_ with some options, such
as ``-v``. The ``CHECKING_DIR``variable restrict the test on the given
directory. For example:

.. code:: bash

  make LITFLAGS=--verbose CHECKING_DIR=range clean check

To run the tests on several default compilers (G++, Clang...) and cleaning
in the between, try:

.. code:: bash

  make check-compilers


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
