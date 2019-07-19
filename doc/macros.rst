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

  Look at the macro ``TRISYCL_NO_BARRIER`` to have a better
  performance on some applications.

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


``TRISYCL_NO_BARRIER``:

  When defined, speed-up OpenMP execution on CPU when the program do
  not use barriers.

  This avoid using in OpenMP mode 1 CPU thread per work-item which is
  required normally to emulate on CPU the GPU-esque programming model
  of independent threads synchronized with various kind of barriers.
  Since in triSYCL the host device is executed by the pure C++ runtime
  without any compiler support, we cannot use some de-SPMD-ization
  techniques to remove some useless barriers and reconstruct some
  vectorized loops with a compiler for good CPU execution. But this
  pure runtime execution has a lot of advantage to handle SYCL
  extensions, virtualization, debugging and emulation, so there is no
  plan to change this behavior in triSYCL.

  So you can use this macro before including the triSYCL header at a
  translation-unit level to speed-up host kernels if you know that you
  will not use barriers;


``TRISYCL_OPENCL``:

  When defined, provide some support for OpenCL interoperability
  mode. This requires for example Boost.Compute and OpenCL support to
  be available on the host and target, typically linking with an
  OpenCL library.

  By default triSYCL does not use OpenCL, so the kernels are run on
  the CPU.


``TRISYCL_TBB``:

  Use the TBB back-end to execute in parallel on the available CPU
  threads the kernels launched on host queues, instead of using OpenMP
  or just a sequential execution.

  Note that the TBB back-end does not support barriers inside a
  ``parallel_for``, but anyway they are performance evil on CPU in our
  case because we do not have a compiler to remove useless barriers.
  Everybody should use on any device the more modern SYCL higher-level
  hierarchical parallelism instead of the old-style thread spaghetti
  with barriers common on GPU;


``TRISYCL_TRACE_KERNEL``:

  Trace the kernel execution.

  This typically requires to have ``BOOST_LOG_DYN_LINK`` defined and
  linked with ``boost_log`` and ``pthread`` libraries on Linux.

``TRISYCL_USE_OPENCL_ND_RANGE``:

  When defined SYCL ``parallel_for`` kernels will be executed using an OpenCL
  ND range rather than en-queuing a single work-item task and looping over the
  range inside of it it. When defined OpenCL builtins (``get_global_id``,
  ``get_local_id``, etc.) are also used to generate SYCL index and range class
  data (``id``, ``range``, etc.) This is currently a work in progress feature.

..
    # Some Emacs stuff:
    ### Local Variables:
    ### mode: rst
    ### minor-mode: flyspell
    ### ispell-local-dictionary: "american"
    ### End:
