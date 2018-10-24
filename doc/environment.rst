====================================
 Environment variables with triSYCL
====================================

triSYCL is not directly using environment variables but uses some
libraries that do.

Of course the generic environment variables of the operating system
have impacts on the execution, for example by selecting the right
dynamic libraries and so on.

Boost.Compute
=============

In this library https://www.boost.org/doc/libs/release/libs/compute
used to support OpenCL, there are some environment variables described
in
https://github.com/boostorg/compute/blob/master/include/boost/compute/system.hppto
that influence the default device used by the implementation:

``BOOST_COMPUTE_DEFAULT_DEVICE``
  name of the compute device (e.g. ``"GTX TITAN"``)

``BOOST_COMPUTE_DEFAULT_DEVICE_TYPE``
  type of the compute device (e.g. ``"GPU"`` or ``"CPU"``)

``BOOST_COMPUTE_DEFAULT_PLATFORM``
  name of the platform (e.g. ``"NVIDIA CUDA"``)

``BOOST_COMPUTE_DEFAULT_VENDOR``
  name of the device vendor (e.g. ``"Xilinx"``)

``BOOST_COMPUTE_DEFAULT_ENFORCE``
  If this is set to ``"1"``, then throw a ``no_device_found()`` exception
  if any of the above environment variables is set, but a matching
  device was not found.

There are also some macros such as ``BOOST_COMPUTE_USE_OFFLINE_CACHE``
than can be used to cache the kernels instead of recompiling them
every time.


OpenMP
======

triSYCL can use OpenMP to speed up execution of the kernels on CPU.

Look at the documentation of the implementation you are using to see
the list of environment variables.

For example with GCC ``libgomp`` look at
https://gcc.gnu.org/onlinedocs/libgomp/Environment-Variables.html
describing for example among others:

``OMP_NUM_THREADS``
  Specifies the number of threads to use


OpenCL backend
==============

Refer to the OpenCL implementation you are using for the useful
environment variables.

..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
