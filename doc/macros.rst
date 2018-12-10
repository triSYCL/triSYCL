======================================
 Pre-processor macros used in triSYCL
======================================

triSYCL can be configured to some pre-processor macros.

They are typically passed to the compiler by the build system.

``CL_SYCL_LANGUAGE_VERSION``:

  When defined, set the expected SYCL version to be followed. The
  default value in triSYCL is ``121``.


``TRISYCL_CL_LANGUAGE_VERSION``:

    When defined, set the expected triSYCL version to be followed. The
    default value is ``121``.

    This is not used yet.


``_OPENMP``:

  When defined, OpenMP support is used to speed-up kernels on the CPU.

  Note this is not a macro expected to be set directly by the
  programmer, but by the compiler when compiling with an OpenMP mode,
  such as with ``-fopenmp``.

  Note that if you do not use OpenMP and the asynchronous mode is
  used, you need to link with the ``pthread`` library in a Unix
  environment.


``TRISYCL_DEBUG``:

  When defined, triSYCL run in debug mode with a lot of verbosity.

  This typically requires to have ``BOOST_LOG_DYN_LINK`` defined and
  linked with ``boost_log`` and ``pthread`` libraries on Linux.


``TRISYCL_DEBUG_STRUCTORS``:

  When defined also with ``TRISYCL_DEBUG``, the creation and
  destruction of various triSYCL objects are traced.


``TRISYCL_NO_ASYNC``:

  When defined, use synchronous kernel execution, instead of the
  standard asynchronous in SYCL with a graph of task. This is used for
  debug purpose.

  The execution is tried when a submit is done. If the dependencies
  are not ready for the kernel to start, a dead-lock occurs.


``TRISYCL_OPENCL``:

  When defined, provide some support for OpenCL interoperability
  mode. This requires for example Boost.Compute and OpenCL support to
  be available on the host and target, typically linking with an
  OpenCL library.

  By default triSYCL does not use OpenCL, so the kernels are run on
  the CPU.


``TRISYCL_TRACE_KERNEL``:

  Trace the kernel execution.

  This typically requires to have ``BOOST_LOG_DYN_LINK`` defined and
  linked with ``boost_log`` and ``pthread`` libraries on Linux.

``TRISYCL_USE_OPENCL_ND_RANGE``:

  When defined it will make use of OpenCL's nd_range_kernel and builtins when
  using parallel_for, currently a work in progress feature.

..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
